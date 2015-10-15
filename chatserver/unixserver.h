#pragma once

#include <sys/epoll.h>
#include "CommandCenter.h"

class ChatServer
{
public:
	ChatServer();
	~ChatServer();
	HERRCODE start();
	void wait();
	bool quit();

private:
	HERRCODE initSock(const char* port);
	void threadFun();

private:
	int listenSock_;
	int udpSock_;
	int epfd_;
	std::vector<std::thread> threads_;
	std::atomic<bool> quit_;
	std::atomic<int> acceptedRequest_;
	std::vector<epoll_event> events_;
	CommandCenter cmdCenter_;
};
