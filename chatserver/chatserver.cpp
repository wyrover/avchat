// chatserver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ChatServer.h"
#include "Utils.h"
#include "user.h"
#include "ServerContext.h"
#include "client.h"
#include "../common/errcode.h"
#include "../common/Utils.h"
#include "../common/ChatOverlappedData.h"
#include "../common/FileRequestCommand.h"
#include "../common/NetConstants.h"
#include "../common/MessageCommand.h"
#include "../common/LoginCommand.h"
#include "../common/SockStream.h"
#include "../common/trace.h"

#define LOG_LOGIN 1
#define LOG_IOCP 0
const static int kPort = 2333;
const static int kMaxAcceptRequest = 1000;
LPFN_GETACCEPTEXSOCKADDRS ChatServer::lpfnGetAcceptExSockaddrs = nullptr;
LPFN_ACCEPTEX ChatServer::lpfnAcceptEx = nullptr;

ChatServer::ChatServer()
	: cmdCenter_(this)
{
	quit_ = false;
	acceptRequest_ = 0;
	acceptedRequest_ = 0;
}

ChatServer::~ChatServer()
{
}

HERRCODE ChatServer::start()
{
	auto hr = ServerContext::getInstance()->init();
	if (hr != H_OK)
		return hr;

	hr = initSock();
	return hr;
}

int ChatServer::initSock()
{
	listenSock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock_ == INVALID_SOCKET)
		return H_NETWORK_ERROR;
	udpSock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSock_ == INVALID_SOCKET)
		return H_NETWORK_ERROR;

	hComp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hComp_ == NULL)
		return H_SERVER_ERROR;

	HANDLE hComp2 = CreateIoCompletionPort((HANDLE)listenSock_, hComp_, kTcpCompKey, 0);
	if (hComp2 != hComp_)
		return H_SERVER_ERROR;

	hComp2 = CreateIoCompletionPort((HANDLE)udpSock_, hComp_, kUdpCompKey, 0);
	if (hComp2 != hComp_)
		return H_SERVER_ERROR;
	
	sockaddr_in addr = { 0 };
	addr.sin_port = htons(kPort);
	addr.sin_family = AF_INET;
	IN_ADDR in_addr;
	inet_pton(AF_INET, "127.0.0.1", &in_addr);
	addr.sin_addr = in_addr;

	int ret = bind(listenSock_, (const sockaddr*)&addr, sizeof(sockaddr_in));
	if (ret != 0) {
		return H_NETWORK_ERROR;
	}

	ret = bind(udpSock_, (const sockaddr*)&addr, sizeof(sockaddr_in));
	if (ret != 0) {
		return H_NETWORK_ERROR;
	}

	ret = listen(listenSock_, SOMAXCONN);
	if (ret != 0) {
		return H_NETWORK_ERROR;
	}

	GUID guidAcceptEx = WSAID_ACCEPTEX;
	GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	DWORD bytes;
	ret = WSAIoctl(listenSock_, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx,
		sizeof(GUID), &lpfnAcceptEx, sizeof(void*), &bytes, NULL, NULL);
	if (!(ret == 0 && lpfnAcceptEx)) {
		return H_NETWORK_ERROR;
	}
	ret = WSAIoctl(listenSock_, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidGetAcceptExSockaddrs,
		sizeof(GUID), &lpfnGetAcceptExSockaddrs, sizeof(void*), &bytes, NULL, NULL);
	if (!(ret == 0 && lpfnGetAcceptExSockaddrs)) {
		return H_NETWORK_ERROR;
	}

	int threadCount = base::Utils::GetCpuCount() * 2;
	for (int i = 0; i < threadCount; ++i){
		threads_.push_back(std::thread(&ChatServer::threadFun, this));
	}
	ChatOverlappedData* overlap = new ChatOverlappedData(net::kAction_Recv);
	overlap->setSocket(udpSock_);
	base::Utils::QueueRecvCmdRequest(udpSock_, hComp_, kUdpCompKey);
	return H_OK;
}

void ChatServer::threadFun()
{
	TRACE_IF(LOG_IOCP, "chatserver::run %d\n", GetCurrentThreadId());
	while (!quit_) {
		if (acceptRequest_ < kMaxAcceptRequest) {
			ChatOverlappedData* overlap = new ChatOverlappedData(net::kAction_Accept);
			SOCKET acceptSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			overlap->setSocket(acceptSock);
			buffer& recvBuf = overlap->getBuf();
			BOOL ret = lpfnAcceptEx(listenSock_, acceptSock, recvBuf.data(), recvBuf.size() - (sizeof(sockaddr_in) + 16) * 2,
				sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, overlap);
			DWORD errCode = GetLastError();
			if (ret) {
				TRACE_IF(LOG_IOCP,"get an accepted sync\n");
			} else if (errCode == ERROR_IO_PENDING) {
				acceptRequest_++;
				TRACE_IF(LOG_IOCP,"%d queued an accept request\n", acceptRequest_);
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
	TRACE_IF(LOG_IOCP, "try to queue completion status\n");
	DWORD bytes;
	ULONG_PTR key; ChatOverlappedData* ol;
	if (!GetQueuedCompletionStatus(hComp_, &bytes, &key, (LPOVERLAPPED*)&ol, 0)) {
		TRACE_IF(LOG_IOCP, "no queued completion status\n");
		return false;
	}
	int type = ol->getNetType();
	if (key == kTcpCompKey) {
		TRACE_IF(LOG_IOCP, "get queued completion status type = %d\n", type);
		if (type == net::kAction_Accept) {
			onAccept(ol, bytes, key);
		} else if (type == net::kAction_Recv) {
			TRACE("get recv request result for ol %x\n", ol);
			onRecv(ol, bytes, key);
		} else if (type == net::kAction_Send) {

		}
	} else if (key == kUdpCompKey) {
		if (type == net::kAction_Recv) {
		}
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

	HANDLE hComp2 = CreateIoCompletionPort((HANDLE)ol->getSocket(), hComp_, kTcpCompKey, 0);
	assert(hComp2 == hComp_);

	char str[20];
	inet_ntop(AF_INET, &((sockaddr_in*)remoteAddr)->sin_addr, str, 20);
	acceptRequest_--;
	acceptedRequest_++;
	TRACE_IF(LOG_LOGIN, "%d recv connection from %s, thread id = %d\n", acceptedRequest_, str, GetCurrentThreadId());
	onRecv(ol, bytes, key);
}

void ChatServer::onRecv(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key)
{
	cmdCenter_.fill(ol->getSocket(), ol->getBuf().data(), bytes);
}

bool ChatServer::quit()
{
	quit_ = true;
	return true;
}

void ChatServer::wait()
{
	for (auto& thread : threads_){
		thread.join();
	}
}

HANDLE ChatServer::getCompletePortHandle()
{
	return hComp_;
}
