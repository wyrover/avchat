#pragma once

#include "ClientManager.h"
#include "CommandCenter.h"
class ChatOverlappedData;
class ChatCommand;
class MessageCommand;
class LoginCommand;
class FileRequestCommand;

class ChatServer
{
public:
	ChatServer();
	~ChatServer();
	HERRCODE init();
	void run();
	bool quit();

private:
	HERRCODE initWinsock();
	HERRCODE initListen();

	bool queueCompletionStatus();
	void onAccept(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key);
	void onRecv(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key);

private:
	SOCKET listenSock_;
	HANDLE hComp_;
	std::atomic<bool> quit_;
	std::atomic<int>  acceptRequest_;
	std::atomic<int> acceptedRequest_;
	CommandCenter cmdCenter_;
	static LPFN_ACCEPTEX        lpfnAcceptEx;
	static LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;
};