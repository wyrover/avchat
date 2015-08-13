#include "stdafx.h"
#include "../common/SockStream.h"
#include "../common/NetConstants.h"
#include "../common/ChatOverlappedData.h"
#include "../common/trace.h"
#include "../common/FileUtils.h"
#include "Utils.h"
#include "CommandCenter.h"
#include "IChatClientController.h"
#include "ImageMessageForSend.h"
#include "ImageMessageForRecv.h"
#include "chatclient.h"

using client::Utils;

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
			auto size = stream.getInt();
			auto sender = stream.getString();
			auto recver = stream.getString();
			auto timestamp = stream.getInt64();
			auto message = stream.getString();
			onCmdMessage(socket, sender, recver, timestamp, message);
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
		default:
			assert(false);
			break;
	}
	return 0;
}

void CommandCenter::onCmdMessage(SOCKET socket, const std::wstring& sender, const std::wstring& recver,
	 time_t timestamp, const std::wstring& message)
{
	if (message.find(L"<img") == -1) {
		controller_->onNewMessage(sender, recver, timestamp, message);
	} else {
		ImageMessageForRecv* msg = new ImageMessageForRecv;
		msg->setRawMessage(message, sender, recver, timestamp);
		auto list = msg->getNeedDownloadFileList(client_->getImageDir());
		if (list.empty()) {
			delete msg;
			controller_->onNewMessage(sender, recver, timestamp, message);
			return;
		}
		SockStream ss;
		ss.writeInt(net::kCommandType_FileDownload);
		ss.writeInt(0); //dummy size
		ss.writeInt((int)msg);
		ss.writeStringVec(list);
		ss.flushSize();
		queueSendRequest(socket, ss);
	}
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
	WSABUF wsaBuf;
	wsaBuf.buf = stream.getBuf();
	wsaBuf.len = stream.getSize();
	ChatOverlappedData* ol = new ChatOverlappedData(net::kAction_Send);
	int ret = WSASend(socket, &wsaBuf, 1, NULL, 0, ol, NULL);
	int errcode = WSAGetLastError();
	if (ret != 0) {
		assert(errcode == WSA_IO_PENDING);
	}
}

void CommandCenter::queueRecvCmdRequest(SOCKET socket)
{
	auto ol = new ChatOverlappedData(net::kAction_Recv);
	ol->setSocket(socket);
	buffer& buf = ol->getBuf();
	WSABUF wsaBuf;
	wsaBuf.buf = buf.data();
	wsaBuf.len = buf.size();
	DWORD flags = 0;
	int err = WSARecv(socket, &wsaBuf, 1, NULL, &flags, ol, NULL);
	int errcode = WSAGetLastError();
	if (err != 0) {
		//TRACE_IF(LOG_IOCP, "wsarecv error %d\n", errcode);
		assert(errcode == WSA_IO_PENDING);
	}
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
	SockStream stream;
	stream.writeInt(net::kCommandType_Message);
	stream.writeInt(0);
	stream.writeString(client_->getUsername());
	stream.writeString(msg->getRecver());
	stream.writeInt64(msg->getTimeStamp());
	stream.writeString(message);
	stream.flushSize();
	queueSendRequest(socket, stream);
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