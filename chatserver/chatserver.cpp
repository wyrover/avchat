// chatserver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ChatServer.h"
#include "Utils.h"
#include "../common/ChatOverlappedData.h"
#include "../common/NetConstants.h"
#include "../common/MessageCommand.h"
#include "../common/LoginCommand.h"
#include "../common/SockStream.h"
#include "../common/trace.h"

#define LOG 0
const static int kCompKey = 233;
const static int kPort = 2333;
const static int kMaxAcceptRequest = 1000;
LPFN_GETACCEPTEXSOCKADDRS ChatServer::lpfnGetAcceptExSockaddrs = nullptr;
LPFN_ACCEPTEX ChatServer::lpfnAcceptEx = nullptr;

ChatServer::ChatServer()
{
	quit_ = false;
	acceptRequest_ = 0;
	acceptedRequest_ = 0;
}

ChatServer::~ChatServer()
{
	WSACleanup();
}

void ChatServer::init()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	assert(err == 0);

	listenSock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	assert(listenSock_ != INVALID_SOCKET);

	hComp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	assert(hComp_ != NULL);

	HANDLE hComp2 = CreateIoCompletionPort((HANDLE)listenSock_, hComp_, kCompKey, 0);
	assert(hComp2 == hComp_);

	sockaddr_in addr = { 0 };
	addr.sin_port = htons(kPort);
	addr.sin_family = AF_INET;
	IN_ADDR in_addr;
	inet_pton(AF_INET, "127.0.0.1", &in_addr);
	addr.sin_addr = in_addr;

	int ret = bind(listenSock_, (const sockaddr*)&addr, sizeof(sockaddr_in));
	assert(ret == 0);

	ret = listen(listenSock_, SOMAXCONN);
	assert(ret == 0);

	GUID guidAcceptEx = WSAID_ACCEPTEX;
	GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	DWORD bytes;
	ret = WSAIoctl(listenSock_, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx,
		sizeof(GUID), &lpfnAcceptEx, sizeof(void*), &bytes, NULL, NULL);
	assert(ret == 0 && lpfnAcceptEx);
	ret = WSAIoctl(listenSock_, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidGetAcceptExSockaddrs,
		sizeof(GUID), &lpfnGetAcceptExSockaddrs, sizeof(void*), &bytes, NULL, NULL);
	assert(ret == 0 && lpfnGetAcceptExSockaddrs);

	int threadCount = Utils::GetCpuCount() * 2;
	std::vector<std::thread> threads;
	for (int i = 0; i < threadCount; ++i){
		threads.push_back(std::thread(&ChatServer::run, this));
	}
	for (auto& thread : threads){
		thread.join();
	}
}

void ChatServer::run()
{
	TRACE("chatserver::run %d\n", GetCurrentThreadId());
	while (!quit_) {
		if (acceptRequest_ < kMaxAcceptRequest) {
			ChatOverlappedData* overlap = new ChatOverlappedData(net::kNetType_Accept);
			SOCKET acceptSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			overlap->setSocket(acceptSock);
			buffer& recvBuf = overlap->getBuf();
			BOOL ret = lpfnAcceptEx(listenSock_, acceptSock, recvBuf.data(), recvBuf.size() - (sizeof(sockaddr_in) + 16) * 2,
				sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, overlap);
			DWORD errCode = GetLastError();
			if (ret) {
				TRACE_IF(LOG,"get an accepted sync\n");
			} else if (errCode == ERROR_IO_PENDING) {
				acceptRequest_++;
				TRACE_IF(LOG,"%d queued an accept request\n", acceptRequest_);
				queueCompletionStatus();
			} else {
				abort();
			}
		} else {
			if (!queueCompletionStatus()) {
				Sleep(100);
			}
		}
	}
}

bool ChatServer::queueCompletionStatus()
{
	TRACE_IF(LOG,"try to queue completion status\n");
	DWORD bytes;
	ULONG_PTR key; ChatOverlappedData* ol;
	if (!GetQueuedCompletionStatus(hComp_, &bytes, &key, (LPOVERLAPPED*)&ol, 0)) {
		TRACE_IF(LOG,"no queued completion status\n");
		return false;
	}
	int type = ol->getNetType();
	TRACE_IF(LOG, "get queued completion status type = %d\n", type);
	if (type == net::kNetType_Accept) {
		onAccept(ol, bytes, key);
	} else if (type == net::kNetType_Recv) {
		onRecv(ol, bytes, key);
	} else if (type == net::kNetType_Send) {

	} 
	return true; 
}

void ChatServer::onAccept(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key)
{
	sockaddr* localAddr;
	sockaddr* remoteAddr;
	INT localAddrLen, remoteAddrLen;
	buffer& recvBuf = ol->getBuf();
	lpfnGetAcceptExSockaddrs(recvBuf.data(), recvBuf.size() - (sizeof(sockaddr_in) + 16) * 2,
		sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
		&localAddr, &localAddrLen, &remoteAddr, &remoteAddrLen);

	HANDLE hComp2 = CreateIoCompletionPort((HANDLE)ol->getSocket(), hComp_, kCompKey, 0);
	assert(hComp2 == hComp_);

	char str[20];
	inet_ntop(AF_INET, &((sockaddr_in*)remoteAddr)->sin_addr, str, 20);
	acceptRequest_--;
	acceptedRequest_++;
	TRACE("%d recv connection from %s, thread id = %d\n", acceptedRequest_, str, GetCurrentThreadId());
	onRecv(ol, bytes, key);
}

void ChatServer::onRecv(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key)
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

void ChatServer::addSocketMap(SOCKET so, ClientState* cs) 
{
	std::lock_guard<std::mutex> g(mapMutex_);
	connMap_[so] = *cs;
}

bool ChatServer::getClientByUsername(const std::wstring& username, ClientState* cs)
{
	std::lock_guard<std::mutex> g(mapMutex_);

	if (!connMap_.count(username))
		return false;
	*cs = connMap_[username];
	return true;
}

void ChatServer::removeClientByUsername(const std::wstring& username)
{
	std::lock_guard<std::mutex> g(mapMutex_);
	connMap_.erase(username);
}

void ChatServer::updateUserlist()
{
	SockStream stream;
	stream.writeInt(net::kCommandType_UserList);
	stream.writeInt(0);
	std::lock_guard<std::mutex> g(mapMutex_);
	stream.writeInt(connMap_.size());
	for (auto& item : connMap_) {
		stream.writeString(item.second.username);
	}
	auto pSize = stream.getBuff() + 4;
	*pSize = stream.getSize();
	for (auto& item : connMap_) {
		auto cs = item.second;
		send(&cs, stream.getBuff(), stream.getSize());
	}
}

void ChatServer::send(ClientState* cs, char* buff, int len)
{
	assert(cs->recvOl);
	if (!cs->sendOl) {
		cs->sendOl = new ChatOverlappedData(net::kNetType_Send);
		cs->sendOl->setSocket(cs->recvOl->getSocket());
	}
	WSABUF wsaBuf;
	wsaBuf.buf = buff;
	wsaBuf.len = len;
	assert(cs->sendOl && cs->sendOl->getNetType() == net::kNetType_Send);
	int ret = WSASend(cs->sendOl->getSocket(), &wsaBuf, 1, NULL, 0, cs->sendOl, NULL);
	if (ret != 0) {
		assert(WSAGetLastError() == WSA_IO_PENDING);
	}
}

bool ChatServer::quit()
{
	quit_ = true;
	return true;
}
void ChatServer::onCmdLogin(LoginCommand* loginCmd, ClientState* cs)
{
	std::wstring username = loginCmd->getUsername();
	std::wstring password = loginCmd->getPassword();
	addSocketMap(username, cs);
	SockStream stream;
	stream.writeInt(net::kCommandType_LoginAck);
	stream.writeInt(0);
	stream.writeInt(1);
	auto pSize = stream.getBuff() + 4;
	*pSize = stream.getSize();
	send(cs, stream.getBuff(), stream.getSize());
	updateUserlist();
}

void ChatServer::queueRecvCmdRequest(ChatOverlappedData* ol)
{
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

void ChatServer::dispatchCommand(ChatOverlappedData* ol, ChatCommand* cmd)
{
	int type = cmd->getType();
	switch (type) {
		case net::kCommandType_Login:
		{
			onCmdLogin(dynamic_cast<LoginCommand*>(cmd), ol);
			break;
		}
		case net::kCommandType_Message:
		{
			onCmdMessage(dynamic_cast<MessageCommand*>(cmd), ol);
			break;
		}
		default:
			break;
	}
}

void ChatServer::parseCommand(ChatOverlappedData* ol, char* recvBuf, int& bytes, int& neededSize,
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

ChatCommand* ChatServer::getCommand(char* recvBuf, int bytes, buffer& cmdBuf)
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
			auto cmd = LoginCommand::Parse(&stream);
			return cmd;
		}
		case net::kCommandType_Message:
		{
			auto cmd = MessageCommand::Parse(&stream);
			return cmd;
		}
		default:
			break;
	}
	return nullptr;
}

void ChatServer::onCmdMessage(MessageCommand* messageCmd, ClientState* cs)
{
	auto sender = messageCmd->getSender();
	auto recver = messageCmd->getReceiver();
	auto msg = messageCmd->getMessage();
	SockStream stream;
	stream.writeInt(net::kCommandType_Message);
	stream.writeInt(0);
	stream.writeString(sender);
	stream.writeString(msg);
	auto pSize = stream.getBuff() + 4;
	*pSize = stream.getSize();
	if (recver == L"all") {
		std::lock_guard<std::mutex> g(mapMutex_);
		for (auto& item : connMap_) {
			auto cs = item.second;
			send(&cs, stream.getBuff(), stream.getSize());
		}
	} else {
		ClientState cs;
		if (getClientByUsername(recver, &cs)) {
			send(&cs, stream.getBuff(), stream.getSize());
		}
	}
}

bool ChatServer::chatDataToClientState(ChatOverlappedData* ol, ClientState* cs)
{
}
