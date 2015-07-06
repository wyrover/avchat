// chatserver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ChatServer.h"
#include "Utils.h"
#include "ChatOverlappedData.h"
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
			ChatOverlappedData* overlap = new ChatOverlappedData(ChatOverlappedData::kType_Accept);
			SOCKET acceptSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			char* acceptBuff = new char[1024];
			overlap->setAcceptBuffer(acceptBuff);
			overlap->setAcceptSock(acceptSock);
			BOOL ret = lpfnAcceptEx(listenSock_, acceptSock, acceptBuff, 1024 - (sizeof(sockaddr_in) + 16) * 2,
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
	ULONG_PTR key;
	ChatOverlappedData* ol;
	if (!GetQueuedCompletionStatus(hComp_, &bytes, &key, (LPOVERLAPPED*)&ol, 0)) {
		TRACE_IF(LOG,"no queued completion status\n");
		return false;
	}
	int type = ol->getType();
	TRACE_IF(LOG,"get queued completion status type = %d\n", type);
	if (type == ChatOverlappedData::kType_Accept) {
		onAccept(ol, bytes, key);
	} else if (type == ChatOverlappedData::kType_Recv) {

	} else if (type == ChatOverlappedData::kType_Send) {

	} else if (type == ChatOverlappedData::kType_AcceptAck) {
		TRACE_IF(LOG,"login ack done\n");
	}
	return true; 
}

void ChatServer::onAccept(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key)
{
	sockaddr* localAddr;
	sockaddr* remoteAddr;
	INT localAddrLen, remoteAddrLen;
	lpfnGetAcceptExSockaddrs(ol->getAccpetBuffer(), 1024 - (sizeof(sockaddr_in) + 16) * 2, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
		&localAddr, &localAddrLen, &remoteAddr, &remoteAddrLen);
	char str[20];
	inet_ntop(AF_INET, &((sockaddr_in*)remoteAddr)->sin_addr, str, 20);
	acceptRequest_--;
	SockStream stream(ol->getAccpetBuffer(), 1024 - (sizeof(sockaddr_in) + 16) * 2);
	int type = stream.getInt();
	assert(type == ChatCommand::kAction_Login);
	int size = stream.getInt();
	auto username = stream.getString();
	auto password = stream.getString();
	acceptedRequest_++;
	TRACE_IF(LOG,"%d new connection from remote: %s, username: %S, password: %S\n", acceptedRequest_, str, username.c_str(), password.c_str());
	TRACE("%d thread id %d\n", acceptedRequest_, GetCurrentThreadId());
	addSocketMap(username, ol);

	char ret = 1;
	ol->setType(ChatOverlappedData::kType_AcceptAck);
	send(ol->getAcceptSock(), &ret, 1, ol);
}

void ChatServer::addSocketMap(const std::wstring& username, ChatOverlappedData* ol)
{
	std::lock_guard<std::mutex> g(mapMutex_);
	connMap_[username] = ol;
}

SOCKET ChatServer::getSocketByUsername(const std::wstring& username)
{
	std::lock_guard<std::mutex> g(mapMutex_);
	return connMap_[username]->getAcceptSock();
}

void ChatServer::removeSocketByUsername(const std::wstring& username)
{
	std::lock_guard<std::mutex> g(mapMutex_);
	connMap_.erase(username);
}

void ChatServer::send(SOCKET socket, char* buff, int len, ChatOverlappedData* ol)
{
	WSABUF wsaBuf;
	wsaBuf.buf = buff;
	wsaBuf.len = len;
	int ret = WSASend(socket, &wsaBuf, 1, NULL, 0, ol, NULL);
	if (ret != 0) {
		assert(WSAGetLastError() == WSA_IO_PENDING);
	}
}

bool ChatServer::quit()
{
	quit_ = true;
	return true;
}
