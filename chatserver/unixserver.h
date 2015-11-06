#pragma once

#include <sys/event.h>
#include <atomic>
#include <vector>
#include <thread>
#include <mutex>
#include "CommandCenter.h"

class ChatServer
{
	public:
		ChatServer();
		~ChatServer();
		HERRCODE start();
		void wait();
		bool quit();
		HERRCODE queueSendRequest(SOCKET sock, SockStream& stream);

	private:
		HERRCODE initSock(const std::string& ip, const std::string& dataPort,
				const std::string& holePort);
		void threadFun();
		bool sendRequest(SOCKET sock, int len);
		bool setWatchOut(SOCKET sock, bool watch);
		int acceptConnections(SOCKET sock);

	private:
		int listenSock_;
		int udpHoleSock_;
		int tcpHoleSock_;
		std::vector<std::thread> threads_;
		std::atomic<bool> quit_;
		std::atomic<int> acceptedRequest_;
		std::map<SOCKET, buffer> sendBuf_;
		std::mutex sendMutex_;
		CommandCenter cmdCenter_;
		int kq_;
};
