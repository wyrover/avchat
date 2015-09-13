#include "stdafx.h"
#include "../common/SockStream.h"
#include "../common/NetConstants.h"
#include "../common/ChatOverlappedData.h"
#include "../common/trace.h"
#include "../common/FileUtils.h"
#include "../common/Utils.h"
#include "Utils.h"
#include "CommandCenter.h"
#include "IChatClientController.h"
#include "ImageMessageForSend.h"
#include "ImageMessageForRecv.h"
#include "chatclient.h"
#include "ErrorManager.h"

using avc::Utils;
namespace avc
{
	CommandCenter::CommandCenter()
	{
		controller_ = nullptr;
	}

	CommandCenter::~CommandCenter()
	{
	}

	int CommandCenter::fill(SOCKET socket, char* inBuf, int inLen)
	{
		if (inLen == 0)
			return 0;
		auto& cmdInfo = cmdMap_[socket];
		if (cmdInfo.fNeededLen == -1) {
			if (cmdInfo.fBuf.size() + inLen < 8) {
				cmdInfo.fBuf.append(inBuf, inLen);
				queueRecvCmdRequest(socket);
				return 0;
			} else {
				int needLen = 8 - cmdInfo.fBuf.size();
				cmdInfo.fBuf.append(inBuf, needLen);
				SockStream stream(cmdInfo.fBuf.data(), cmdInfo.fBuf.size());
				stream.getInt(); // type
				cmdInfo.fNeededLen = stream.getInt() - 8;
				inBuf += needLen;
				inLen -= needLen;
			}
		}

		if (inLen >= cmdInfo.fNeededLen) {
			handleCommand(socket, cmdInfo.fBuf, inBuf, inLen);
			inBuf += cmdInfo.fNeededLen;
			inLen -= cmdInfo.fNeededLen;
			cmdInfo.fNeededLen = -1;
			cmdInfo.fBuf.clear();
			if (inLen == 0) {
				queueRecvCmdRequest(socket);
				return 0;
			} else {
				return fill(socket, inBuf, inLen);
			}
		} else {
			cmdInfo.fBuf.append(inBuf, inLen);
			cmdInfo.fNeededLen -= inLen;
			queueRecvCmdRequest(socket);
		}
		return 0;
	}

	int CommandCenter::handleCommand(SOCKET socket, buffer& cmdBuf, char* inBuf, int inLen)
	{
		char* buf = nullptr;
		int len = 0;
		if (cmdBuf.empty()) {
			buf = inBuf;
			len = inLen;
		} else {
			cmdBuf.append(inBuf, inLen);
			buf = cmdBuf.data();
			len = cmdBuf.size();
		}
		SockStream stream(buf, len);
		int type = stream.getInt();
		switch (type) {
			case net::kCommandType_Message:
			{

				onCmdMessage(socket, stream);
				break;
			}
			case net::kCommandType_UserList:
			{
				int size = stream.getInt();
				auto userVec = stream.getStringVec();
				onCmdUserList(socket, userVec);
				break;
			}
			case net::kCommandType_FileExistsAck:
			{
				auto size = stream.getInt();
				ImageMessageForSend* msg = (ImageMessageForSend*)stream.getInt();
				auto urlList = stream.getStringVec();
				onCmdFileCheckAck(socket, msg, urlList);
				break;
			}
			case net::kCommandType_FileUploadAck:
			{
				auto size = stream.getInt();
				ImageMessageForSend* msg = (ImageMessageForSend*)stream.getInt();
				auto urlList = stream.getStringVec();
				onCmdFileUploadAck(socket, msg, urlList);
				break;
			}
			case net::kCommandType_FileDownloadAck:
			{
				onCmdFileDownloadAck(socket, stream);
				break;
			}
			case net::kCommandType_MessageAck:
			{
				onCmdMessageAck(socket, stream);
				break;
			}
			case net::kCommandType_FileTransferRequest:
			{
				onCmdFileTransferRequest(socket, stream);
				break;
			}
			case net::kCommandType_FileTransferRequestAck:
			{
				onCmdFileTransferRequestAck(socket, stream);
				break;
			}
			case net::kCommandType_BuildPath:
			{
				client_->peerMan().onCmdBuildP2pPath(socket, stream);
				break;
			}
			case net::kCommandType_PeerSync:
			{
				client_->peerMan().onCmdPeerSync(socket, stream);
				break;
			}
			case net::kCommandType_BuildPathAck:
			{
				client_->peerMan().onCmdBuildP2pPathAck(socket, stream);
				break;
			}
			default:
				assert(false);
				break;
		}
		return 0;
	}

	HERRCODE CommandCenter::onCmdMessage(SOCKET socket, SockStream& is)
	{
		int size;
		std::wstring sender;
		std::wstring recver;
		int64_t timestamp;
		std::wstring message;
		try {
			size = is.getInt();
			if (size != is.getSize())
				return H_INVALID_PACKAGE;
			sender = is.getString();
			recver = is.getString();
			timestamp = is.getInt64();
			message = is.getString();
		} catch (std::out_of_range& e) {
			return H_INVALID_PACKAGE;
		}

		SockStream os;
		os.writeInt(net::kCommandType_MessageAck);
		os.writeInt(0); //dummy size
		os.writeInt64(timestamp);
		os.writeString(recver);
		os.flushSize();
		queueSendRequest(socket, os);

		if (message.find(L"<img") == -1) {
			controller_->onNewMessage(sender, recver, timestamp, message);
		} else {
			ImageMessageForRecv* msg = new ImageMessageForRecv;
			msg->setRawMessage(message, sender, recver, timestamp);
			auto list = msg->getNeedDownloadFileList(client_->getImageDir());
			if (list.empty()) {
				delete msg;
				controller_->onNewMessage(sender, recver, timestamp, message);
				return H_OK;
			}
			SockStream os;
			os.writeInt(net::kCommandType_FileDownload);
			os.writeInt(0); //dummy size
			os.writeInt((int)msg);
			os.writeStringVec(list);
			os.flushSize();
			queueSendRequest(socket, os);
		}
		return H_OK;
	}

	void CommandCenter::onCmdUserList(SOCKET socket, const std::vector<std::wstring>& userList)
	{
		{
			std::lock_guard<std::recursive_mutex> guard(userMutex_);
			userList_ = userList;
		}
		controller_->onNewUserList();
	}

	void CommandCenter::queueSendRequest(SOCKET socket, SockStream& stream)
	{
		base::Utils::QueueSendRequest(socket, stream, client_->getHandleComp(), ChatClient::kServerKey);
	}

	void CommandCenter::queueRecvCmdRequest(SOCKET socket)
	{
		base::Utils::QueueRecvCmdRequest(socket, client_->getHandleComp(), ChatClient::kServerKey);
	}

	void CommandCenter::setController(ChatClient* client, IChatClientController* controller)
	{
		client_ = client;
		controller_ = controller;
	}

	std::vector<std::wstring> CommandCenter::getUserList()
	{
		std::lock_guard<std::recursive_mutex> guard(userMutex_);
		return userList_;
	}

	void CommandCenter::sendImageMessage(SOCKET socket, ImageMessageForSend* message)
	{
		SockStream ss;
		ss.writeInt(net::kCommandType_FileExists);
		ss.writeInt(0); //dummy size
		ss.writeInt((int)message);
		ss.writeStringVec(message->getHashList());
		ss.flushSize();
		queueSendRequest(socket, ss);
	}

	void CommandCenter::onCmdFileCheckAck(SOCKET socket, ImageMessageForSend* msg, const std::vector<std::wstring>& urlList)
	{
		auto uploadList = msg->getUploadFileList(urlList);
		SockStream ss;
		ss.writeInt(net::kCommandType_FileUpload);
		ss.writeInt(0);
		ss.writeInt((int)msg);
		ss.writeInt(uploadList.size());
		for (auto path : uploadList) {
			auto ext = FileUtils::getFileExt(path);
			ss.writeString(ext);
			buffer outBuf;
			FileUtils::ReadAll(path, outBuf);
			ss.writeBuffer(outBuf);
		}
		ss.flushSize();
		queueSendRequest(socket, ss);
	}

	void CommandCenter::onCmdFileUploadAck(SOCKET socket, ImageMessageForSend* msg,
		const std::vector<std::wstring>& urlList)
	{
		auto message = msg->translateMessage(urlList);
		SockStream os;
		os.writeInt(net::kCommandType_Message);
		os.writeInt(0);
		os.writeString(client_->getEmail());
		os.writeString(msg->getRecver());
		os.writeInt64(msg->getTimeStamp());
		os.writeString(message);
		os.flushSize();
		queueSendRequest(socket, os);
		client_->messageMan().addMessageRequest(msg->getTimeStamp(), msg->getRecver(), msg->getTimeStamp());
	}

	void CommandCenter::onCmdFileDownloadAck(SOCKET socket, SockStream& stream)
	{
		auto size = stream.getInt();
		auto msg = (ImageMessageForRecv*)stream.getInt();
		msg->writeFile(client_->getImageDir(), stream);
		if (controller_) {
			controller_->onNewMessage(msg->getSender(), msg->getRecver(), msg->getTimeStamp(), msg->getRawMessage());
		}
		delete msg;
	}

	HERRCODE CommandCenter::onCmdMessageAck(SOCKET socket, SockStream& stream)
	{
		int size;
		int64_t id;
		std::wstring remoteName;
		try {
			size = stream.getInt();
			if (size != stream.getSize())
				return H_INVALID_PACKAGE;
			id = stream.getInt64();
			remoteName = stream.getString();
		} catch (std::out_of_range& e) {
			return H_INVALID_PACKAGE;
		}
		client_->messageMan().confirmMessageRequest(id, remoteName);
		return H_OK;
	}

	void CommandCenter::onCmdFileTransferRequest(SOCKET socket, SockStream& stream)
	{
		auto size = stream.getInt();
		auto sender = stream.getString();
		auto filename = stream.getString();
		auto filesize = stream.getInt();
		auto timestamp = stream.getInt64();

		client_->controller()->onFileRequest(sender, timestamp, false, filename, filesize);
	}

	HERRCODE CommandCenter::onCmdFileTransferRequestAck(SOCKET socket, SockStream& is)
	{
		try {
			auto size = is.getInt();
			auto sender = is.getString();
			auto timestamp = is.getInt64();
			auto allow = is.getBool();
			client_->controller()->onFileRequestAck(sender, timestamp, allow);
			return H_OK;
		} catch (...) {
			return H_INVALID_PACKAGE;
		}
	}
}
