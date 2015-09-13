#pragma once

#include "CommandCenter.h"
class ChatOverlappedData;

class ChatServer
{
public:
	ChatServer();
	~ChatServer();
	HERRCODE start();
	void wait();
	bool quit();
	HANDLE getCompletePortHandle();
	const static int kTcpCompKey = 233;
	const static int kUdpCompKey = 235;

private:
	HERRCODE initSock();
	void threadFun();

	bool queueCompletionStatus();
	void onAccept(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key);
	void onRecv(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key);

private:
	SOCKET listenSock_;
	SOCKET udpSock_;
	HANDLE hComp_;
	std::vector<std::thread> threads_;
	std::atomic<bool> quit_;
	std::atomic<int>  acceptRequest_;
	std::atomic<int> acceptedRequest_;
	CommandCenter cmdCenter_;
	static LPFN_ACCEPTEX        lpfnAcceptEx;
	static LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;
};