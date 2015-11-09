#include "stdafx.h"
#include <assert.h>
#include <unistd.h>
#include <syslog.h>
#include <sstream>
#include <iomanip>
#include <sys/socket.h>
#include <sys/types.h>
#include "../common/SockStream.h"
#include "../common/NetConstants.h"
#include "../common/trace.h"
#include "../common/FileUtils.h"
#include "../common/Utils.h"
#include "../common/StringUtils.h"

#include "XmlUtils.h"
#include "CommandCenter.h"
#include "IChatClientController.h"
#include "ImageMessageForSend.h"
#include "ImageMessageForRecv.h"
#include "LoginError.h"
#include "ChatClient.h"
#include "ErrorManager.h"

#define LOG_CMD 0


namespace avc
{
	CommandCenter::CommandCenter()
	{
		controller_ = nullptr;
		quit_ = false;
		client_ = nullptr;
	}

	CommandCenter::~CommandCenter()
        {
            quit();
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
			cmdInfo.fBuf.append(inBuf, cmdInfo.fNeededLen);
			postCommand(socket, cmdInfo.fBuf);
			assert(cmdInfo.fBuf.empty());
			inBuf += cmdInfo.fNeededLen;
			inLen -= cmdInfo.fNeededLen;
			cmdInfo.fNeededLen = -1;
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

	void CommandCenter::postCommand(SOCKET socket, buffer& cmdBuf)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		TRACE_IF(LOG_CMD,"post command socket: %d, cmdBuf: %x, len: %d, after len = %d\n", socket, 
				cmdBuf.data(), cmdBuf.size(), cmdQueue_.size() + 1);
		cmdQueue_.push_back(CommandRequest(socket, cmdBuf));
		cv_.notify_one();
	}

	void CommandCenter::start()
	{
		int threadCount = base::Utils::GetCpuCount() * 2;
		quit_ = false;
		for (int i = 0; i < threadCount; ++i) {
			threads_.push_back(std::thread(&CommandCenter::threadFun, this));
		}
	}

	void CommandCenter::threadFun()
	{
		while (!quit_) {
			SOCKET s;
			buffer buf;
			{
				std::unique_lock<std::mutex> lock(mutex_);
                                while (cmdQueue_.empty() && !quit_) {
					cv_.wait(lock);
                                }
                                if (quit_) {
                                    break;
                                }
				TRACE_IF(LOG_CMD,"before pick the queue size = %d\n", cmdQueue_.size());
				s = cmdQueue_.front().fSock;
				buf.swap(cmdQueue_.front().fBuf);
				cmdQueue_.pop_front();
				TRACE_IF(LOG_CMD,"pick command socket: %d, cmdBuf: %x, len: %d, after len = %d\n", s, 
						buf.data(), buf.size(), cmdQueue_.size());

			}
			handleCommand(s, buf);
		}
	}

	void CommandCenter::quit()
	{
                quit_ = true;
                cv_.notify_all();
		for (auto& thread : threads_) {
			thread.join();
		}
	}

	void CommandCenter::queueRecvCmdRequest(SOCKET socket)
	{
	}

	int CommandCenter::handleCommand(SOCKET socket, buffer& cmdBuf)
	{
		char* buf = nullptr;
		int len = 0;
		buf = cmdBuf.data();
		len = cmdBuf.size();
		SockStream stream(buf, len);
		int type = stream.getInt();
		switch (type) {
			case net::kCommandType_ImageMessage:
			{
				onCmdImageMessage(socket, stream);
				break;
			}
			case net::kCommandType_LoginAck:
			{
				syslog(LOG_INFO, "client loginack size = %zu\n", stream.getSize());
				onCmdLoginAck(socket, stream);
				break;
			}
			case net::kCommandType_Message:
			{
				syslog(LOG_INFO, "client message size = %zu\n", stream.getSize());
				onCmdMessage(socket, stream);
				break;
			}
			case net::kCommandType_UserList:
			{
				syslog(LOG_INFO, "client userlist size = %zu\n", stream.getSize());
				onCmdUserList(socket, stream);
				break;
			}
			case net::kCommandType_FileExistsAck:
			{
				onCmdFileCheckAck(socket, stream);
				break;
			}
			case net::kCommandType_FileUploadAck:
			{
				onCmdFileUploadAck(socket, stream);
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

	HERRCODE CommandCenter::onCmdImageMessage(SOCKET socket, SockStream& is)
	{
		int size;
		ImageMessageForSend* message;
		try {
			size = is.getInt();
			if (size != is.getSize())
				return H_INVALID_PACKAGE;
			message = (ImageMessageForSend*)is.getInt64();
		} catch (std::out_of_range& e) {
			return H_INVALID_PACKAGE;
		}

		SockStream os;
		os.writeInt(net::kCommandType_FileExists);
		os.writeInt(0); //dummy size
		os.writeInt64((int64_t)message);
		os.writeStringVec(message->getHashList());
		os.flushSize();
		queueSendRequest(socket, os);
		return H_OK;
	}

	HERRCODE CommandCenter::onCmdFileCheckAck(SOCKET socket, SockStream& is)
	{
		int size;
		ImageMessageForSend* msg;
		std::vector<std::u16string> urlList;
		try {
			size = is.getInt();
			if (size != is.getSize()) {
				return H_INVALID_PACKAGE;
			}
			msg = (ImageMessageForSend*)is.getInt64();
			urlList = is.getStringVec();
		} catch (std::out_of_range& e) {
			return H_INVALID_PACKAGE;
		}

		auto uploadList = msg->getUploadFileList(urlList);
		if (!uploadList.empty()) {
			SockStream os;
			os.writeInt(net::kCommandType_FileUpload);
			os.writeInt(0);
			os.writeInt64((int64_t)msg);
			os.writeInt(uploadList.size());
			for (auto path : uploadList) {
				auto pos = path.rfind('.');
				std::u16string ext;
				if (pos != -1) {
					ext = path.substr(pos + 1);
				}
				os.writeString(ext);
				buffer outBuf;
				auto uPath = su::u16to8(path);
				FileUtils::ReadAll(uPath, outBuf);
				os.writeBuffer(outBuf);
			}
			os.flushSize();
			queueSendRequest(socket, os);
		} else {
			SockStream os;
			os.writeInt(net::kCommandType_Message);
			os.writeInt(0);
			os.writeString(client_->getEmail());
			os.writeString(msg->getRecver());
			os.writeInt64(msg->getTimeStamp());
			auto message = msg->translateMessage(urlList);
			os.writeString(message);
			os.flushSize();
			queueSendRequest(socket, os);
			client_->messageMan().addMessageRequest(msg->getTimeStamp(), msg->getRecver(), msg->getTimeStamp());
		}
		return H_OK;
	}

	HERRCODE CommandCenter::onCmdMessage(SOCKET socket, SockStream& is)
	{
		int size;
		std::u16string sender;
		std::u16string recver;
		int64_t timestamp;
		std::u16string message;
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

		if (message.find(u"<img") == -1) {
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
			os.writeInt64((int64_t)msg);
			os.writeStringVec(list);
			os.flushSize();
			queueSendRequest(socket, os);
		}
		return H_OK;
	}

	HERRCODE CommandCenter::onCmdUserList(SOCKET socket, SockStream &is)
	{
		int size;
		std::vector<std::u16string> userList;
		try {
			size = is.getInt();
			if (size != is.getSize())
				return H_INVALID_PACKAGE;
			userList = is.getStringVec();
		} catch (std::out_of_range& e) {
			return H_INVALID_PACKAGE;
		}
		{
			std::lock_guard<std::recursive_mutex> guard(userMutex_);
			userList_ = userList;
		}
		controller_->onNewUserList();
		return H_OK;
	}

	void CommandCenter::setController(ChatClient* client, IChatClientController* controller)
	{
		client_ = client;
		controller_ = controller;
	}

	std::vector<std::u16string> CommandCenter::getUserList()
	{
		std::lock_guard<std::recursive_mutex> guard(userMutex_);
		return userList_;
	}

	HERRCODE CommandCenter::onCmdFileUploadAck(SOCKET socket, SockStream& is)
	{
		int size;
		ImageMessageForSend* msg;
		std::vector<std::u16string> urlList;
		try {
			size = is.getInt();
			if (size != is.getSize()) {
				return H_INVALID_PACKAGE;
			}
			msg = (ImageMessageForSend*)is.getInt64();
			urlList = is.getStringVec();
		} catch (std::out_of_range& e) {
			return H_INVALID_PACKAGE;
		}

		auto message = msg->translateMessage(urlList);
		syslog(LOG_INFO, "after translate message is %s\n", su::u16to8(message).c_str());
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
		return H_OK;
	}

	void CommandCenter::onCmdFileDownloadAck(SOCKET socket, SockStream& stream)
	{
		auto size = stream.getInt();
		auto msg = (ImageMessageForRecv*)stream.getPtr();
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
		std::u16string remoteName;
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

	HERRCODE CommandCenter::onCmdLoginAck(SOCKET socket, SockStream &is)
	{
		int size;
		int ack;
		int authType;
		std::u16string authKey;
		try {
			size = is.getInt();
			if (size != is.getSize())
				return H_INVALID_PACKAGE;
			ack = is.getInt();
			if (ack != net::kLoginAck_Succeeded) {
				client_->controller()->onChatError(new LoginError());
				return H_AUTH_FAILED;
			}
			authType = is.getInt();
			authKey = is.getString();
			client_->controller()->onChatError(new LoginError(authKey));
			return H_OK;
		} catch (std::out_of_range& e) {
			return H_INVALID_PACKAGE;
		}
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

	void CommandCenter::queueSendRequest(SOCKET socket, SockStream& is)
	{
		client_->queueSendRequest(socket, is);
	}

}

