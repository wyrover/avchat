#pragma once

#include <sys/event.h>
#include <atomic>
#include <vector>
#include <thread>
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
		std::vector<std::thread> threads_;
		std::atomic<bool> quit_;
		std::atomic<int> acceptedRequest_;
		CommandCenter cmdCenter_;

		int kq_;

};
