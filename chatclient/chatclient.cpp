// chatclient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../common/ChatOverlappedData.h"
#include "../common/MessageCommand.h"
#include "../common/SockStream.h"
#include "../common/trace.h"
#include "../common/NetConstants.h"
#include "../common/UserListCommand.h"
#include "../common/LoginCommand.h"
#include "../common/FileUtils.h"
#include "LoginAckCommand.h"
#include "ChatClient.h"
#include <process.h>
#include "Shlwapi.h"

#define LOG 1

static const int kCompKey = 0x1234;

ChatClient::ChatClient(const std::wstring& ipaddr, int port)
{
	InetPton(AF_INET, ipaddr.c_str(), &serverAddr_);
	controller_ = nullptr;
	quit_ = false;
}

ChatClient::~ChatClient()
{
}

bool ChatClient::login(const std::wstring& username, const std::wstring& password)
{
	sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addr = { 0 };
	addr.sin_port = htons(2333);
	addr.sin_family = AF_INET;
	IN_ADDR in_addr;
	inet_pton(AF_INET, "127.0.0.1", &in_addr);
	addr.sin_addr = in_addr;
	int ret = ::connect(sock_, (const sockaddr*)&addr, sizeof(sockaddr_in));
	assert(ret == 0);
	LoginCommand cmd;
	cmd.set(username, password);
	SockStream stream;
	cmd.writeTo(&stream);
	ret = ::send(sock_, stream.getBuf(), stream.getSize(), 0);
	if (ret == SOCKET_ERROR)
		return false;

	buffer buf(100);

	int retSize = ::recv(sock_, buf.data(), buf.size(), 0);
	if (retSize) {
		SockStream ss(buf.data(), retSize);
		assert(net::kCommandType_LoginAck == ss.getInt());
		auto cmd = LoginAckCommand::Parse(&ss);
		TRACE("login result cmd = %d\n", cmd->getResult());
		hComp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		assert(hComp_ != NULL);

		HANDLE hComp2 = CreateIoCompletionPort((HANDLE)sock_, hComp_, kCompKey, 0);
		assert(hComp2 == hComp_);
	}
	userName_ = username;
	return true;
}

void ChatClient::sendMessage(const std::wstring& username, const std::wstring& message, time_t timestamp)
{
	MessageCommand command;
	command.set(userName_, username, message, timestamp);
	SockStream ss;
	command.writeTo(&ss);
	send(sock_, ss.getBuf(), ss.getSize(), nullptr);
}

std::vector<std::wstring> ChatClient::getUserList()
{
	std::lock_guard<std::recursive_mutex> lock(userMutex_);
	return userList_;
}

void ChatClient::run()
{
	TRACE("chatclient::run %d\n", GetCurrentThreadId());
	queueRecvCmdRequest(nullptr);
	while (!quit_) {
		if (!queueCompletionStatus()) {
			Sleep(100);
		}
	}
}

bool ChatClient::isValid()
{
	return valid_;
}

void ChatClient::quit()
{
	quit_ = true;
}

bool ChatClient::queueCompletionStatus()
{
	DWORD bytes;
	ULONG_PTR key; ChatOverlappedData* ol;
	if (!GetQueuedCompletionStatus(hComp_, &bytes, &key, (LPOVERLAPPED*)&ol, 0)) {
		return false;
	}
	int type = ol->getNetType();
	TRACE_IF(LOG, "get queued completion status type = %d\n", type);
	if (type == net::kNetType_Recv) {
		onRecv(ol, bytes, key);
	} else if (type == net::kNetType_Send) {

	}
	return true;
}

void ChatClient::dispatchCommand(ChatOverlappedData* ol, ChatCommand* cmd)
{
	int type = cmd->getType();
	switch (type) {
		case net::kCommandType_LoginAck:
		{
			onCmdLoginAck(dynamic_cast<LoginAckCommand*>(cmd)->getResult(), ol);
			break;
		}
		case net::kCommandType_Message:
		{
			onCmdMessage(dynamic_cast<MessageCommand*>(cmd), ol);
			break;
		}
		case net::kCommandType_UserList:
		{
			onCmdUserList(dynamic_cast<UserListCommand*>(cmd)->getUserList(), ol);
			break;
		}
		case net::kCommandType_FileRequestAck:
		{
			break;
		}
		default:
			break;
	}
}

void ChatClient::parseCommand(ChatOverlappedData* ol, char* recvBuf, int& bytes, int& neededSize,
	std::vector<ChatCommand*>& cmdVec)
{
	if (bytes == 0)
		return;

	buffer& cmdBuf = ol->getCmdBuf();
	int cmdSize = 0;
	if (neededSize == -1)  {
		if (cmdBuf.size() + bytes < 8) {
			cmdBuf.append(recvBuf, bytes);
			queueRecvCmdRequest(ol);
			return;
		} else {
			SockStream stream(ol->getBuf().data(), bytes);
			stream.getInt(); // type
			cmdSize = stream.getInt();
		}
	} else {
		cmdSize = neededSize;
	}

	if (bytes >= cmdSize) {
		ChatCommand* cmd = getCommand(recvBuf, bytes, cmdBuf);
		cmdVec.push_back(cmd);
		neededSize = -1;
		recvBuf += cmdSize;
		bytes -= cmdSize;
		cmdBuf.clear();
		if (bytes != 0)
			parseCommand(ol, recvBuf, bytes, neededSize, cmdVec);
		else
			queueRecvCmdRequest(ol);
	} else {
		cmdBuf.append(recvBuf, bytes);
		neededSize = cmdSize - bytes;
		queueRecvCmdRequest(ol);
	}
}

ChatCommand* ChatClient::getCommand(char* recvBuf, int bytes, buffer& cmdBuf)
{
	char* buf = nullptr;
	int len = 0;
	if (cmdBuf.empty()) {
		buf = recvBuf;
		len = bytes;
	} else {
		cmdBuf.append(recvBuf, bytes);
		buf = cmdBuf.data();
		len = cmdBuf.size();
	}
	SockStream stream(buf, len);
	int type = stream.getInt();
	switch (type) {
		case net::kCommandType_Login:
		{
			auto cmd = LoginAckCommand::Parse(&stream);
			return cmd;
		}
		case net::kCommandType_Message:
		{
			auto cmd = MessageCommand::Parse(&stream);
			return cmd;
		}
		case net::kCommandType_UserList:
		{
			auto cmd = UserListCommand::Parse(&stream);
			return cmd;
		}
		case net::kCommandType_FileRequestAck:
		{
		}
		default: {
			assert(false);
			break;
		}
	}
	return nullptr;
}

void ChatClient::onCmdLoginAck(int ret, ChatOverlappedData* ol)
{
	loggedIn_ = !!ret;
}

void ChatClient::onCmdMessage(MessageCommand* messageCmd, ChatOverlappedData* ol)
{
	auto sender = messageCmd->getSender();
	auto recver = messageCmd->getReceiver();
	auto timestamp = messageCmd->getTimeStamp();
	auto msg = messageCmd->getMessage();
	TRACE(L"new message from %s: %s\n", sender.c_str(), msg.c_str());
	if (controller_) {
		controller_->onNewMessage(sender, recver, timestamp, msg);
	}
}

void ChatClient::onCmdUserList(const std::vector<std::wstring>& userList, ChatOverlappedData* ol)
{
	std::lock_guard<std::recursive_mutex> lock(userMutex_);
	userList_ = userList;
	if (controller_) {
		controller_->onNewUserList();
	}
}

void ChatClient::onRecv(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key)
{
	std::vector<ChatCommand*> cmds;
	int bufSize = bytes;
	parseCommand(ol, ol->getBuf().data(), bufSize, ol->getCmdNeedSize(), cmds);
	for (auto cmd : cmds) {
		dispatchCommand(ol, cmd);
	}
	for (auto cmd : cmds) {
		delete cmd;
	}
}

void ChatClient::queueRecvCmdRequest(ChatOverlappedData* ol)
{
	if (!ol) {
		ol = new ChatOverlappedData(net::kNetType_Recv);
		ol->setSocket(sock_);
	}

	buffer& buf = ol->getBuf();
	WSABUF* wsaBuf = new WSABUF[1];
	wsaBuf[0].buf = buf.data();
	wsaBuf[0].len = buf.size();
	DWORD flags = 0;
	ol->setNetType(net::kNetType_Recv);
	int err = WSARecv(ol->getSocket(), wsaBuf, 1, NULL, &flags, ol, NULL);
	if (err != 0) {
		assert(GetLastError() == WSA_IO_PENDING);
	}
}

void ChatClient::send(SOCKET socket, char* buff, int len, ChatOverlappedData* ol)
{
	if (!ol) {
		ol = new ChatOverlappedData(net::kNetType_Send);
		ol->setSocket(sock_);
	}
	WSABUF wsaBuf;
	wsaBuf.buf = buff;
	wsaBuf.len = len;
	ol->setNetType(net::kNetType_Send);
	int ret = WSASend(socket, &wsaBuf, 1, NULL, 0, ol, NULL);
	if (ret != 0) {
		assert(WSAGetLastError() == WSA_IO_PENDING);
	}
}

static unsigned int __stdcall keke(void* obj) {
	auto client = reinterpret_cast<ChatClient*>(obj);
	client->run();
	return 1;
}

void ChatClient::startThread()
{
	hThread_ = (HANDLE)_beginthreadex(0, 0, keke, this, 0, &threadId_);
}

void ChatClient::setController(IChatClientController* controller)
{
	controller_ = controller;
}

void ChatClient::sendFile(const std::wstring& username, const std::wstring& fileList)
{
	SockStream ss;
	int size = 0;
	size += ss.writeInt(net::kCommandType_FileRequest);
	size += ss.writeInt(0); //dummy size
	size += ss.writeString(userName_);
	size += ss.writeString(username);
	size += ss.writeInt64(time(NULL));
	size += ss.writeBool(FileUtils::DirExists(fileList.c_str()));
	size += ss.writeString(::PathFindFileName(fileList.c_str()));
	auto sizePtr = (int*)(ss.getBuf() + 4);
	*sizePtr = size;
}

std::wstring ChatClient::getUsername()
{
	return userName_;
}
