#include "stdafx.h"
#include "../common/SockStream.h"
#include "../common/NetConstants.h"
#include "../common/ChatOverlappedData.h"
#include "../common/trace.h"
#include "CommandCenter.h"
#include "IChatClientController.h"

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
		default:
			break;
	}
	return 0;
}

void CommandCenter::onCmdMessage(SOCKET socket, const std::wstring& sender, const std::wstring& recver,
	 time_t timestamp, const std::wstring& message)
{
	controller_->onNewMessage(sender, recver, timestamp, message);
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
	ChatOverlappedData* ol = new ChatOverlappedData(net::kNetType_Send);
	int ret = WSASend(socket, &wsaBuf, 1, NULL, 0, ol, NULL);
	int errcode = WSAGetLastError();
	if (ret != 0) {
		assert(errcode == WSA_IO_PENDING);
	}
}

void CommandCenter::queueRecvCmdRequest(SOCKET socket)
{
	auto ol = new ChatOverlappedData(net::kNetType_Recv);
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

void CommandCenter::setController(IChatClientController* controller)
{
	controller_ = controller;
}

std::vector<std::wstring> CommandCenter::getUserList()
{
	std::lock_guard<std::recursive_mutex> guard(userMutex_);
	return userList_;
}
