// chatserver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <thread>
#include "unixserver.h"
#include "Utils.h"
#include "ServerContext.h"
#include "../common/errcode.h"
#include "../common/Utils.h"
#include "../common/NetConstants.h"
#include "../common/SockStream.h"
#include "../common/trace.h"

#define LOG_LOGIN 1
#define LOG_KQUEUE 1

const static int kPort = 2333;
const static int kMaxAcceptRequest = 1000;
const static int kMaxEventsCount = 500;

ChatServer::ChatServer() : cmdCenter_(this)
{
	quit_ = false;
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

	hr = initSock("2333");
	return hr;
}

int ChatServer::initSock(const char* port)
{
	listenSock_ = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSock_ == -1)
		return H_NETWORK_ERROR;

	udpSock_ = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSock_ == -1)
		return H_NETWORK_ERROR;

	addrinfo hints;
	addrinfo *result;
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, port, &hints, &result) != 0) {
		return H_NETWORK_ERROR;
	}

	int rc = -1;
	for (auto rp = result; rp != nullptr; rp = rp->ai_next) {
		rc = bind(listenSock_, rp->ai_addr, rp->ai_addrlen);
		if (rc == 0) {
			break;
		}
	}

	if (rc != 0) {
		return H_NETWORK_ERROR;
	}

	if (base::Utils::MakeSocketNonBlocking(listenSock_) < 0)
		return H_NETWORK_ERROR;

	rc = listen(listenSock_, SOMAXCONN);
	if (rc != 0) {
		return H_NETWORK_ERROR;
	}

	kq_ = kqueue();
	if (kq_ == -1)
		return H_SERVER_ERROR;

	struct kevent ev;
	EV_SET(&ev, listenSock_, EVFILT_READ, EV_ADD, 0, 0, 0);
	if (kevent(kq_, &ev, 1, NULL, 0, NULL)  < 0) {
		return H_NETWORK_ERROR;
	}

	//int threadCount = base::Utils::GetCpuCount() * 2;
	int threadCount = 1;
	for (int i = 0; i < threadCount; ++i){
		threads_.push_back(std::thread(&ChatServer::threadFun, this));
	}
	return H_OK;
}

void ChatServer::threadFun()
{
	TRACE_IF(LOG_KQUEUE, "chatserver::run %d\n", std::this_thread::get_id());
	std::vector<struct kevent> events_;
	events_.resize(kMaxEventsCount);
	struct timespec tmout = { 1, 0 };
	while (!quit_) {
		int eventCount = kevent(kq_, NULL, 0, events_.data(), events_.size(), &tmout);
		if (eventCount > 0) {
			for (int i = 0; i < eventCount; ++i) {
				if (events_[i].flags & EV_EOF) {
					perror("kqueue EV_EOF\n");
					close(events_[i].ident);
					continue;
				}
				if (events_[i].flags & EV_ERROR) {
					fprintf(stderr, "kqueue error: %s\n", strerror(events_[i].data));
					close(events_[i].ident);
					continue;
				}

				if (events_[i].ident == listenSock_) {
					while (true) {
						TRACE("event events flags = %d\n", events_[i].flags);
						sockaddr_in remoteAddr;
						socklen_t addrLen = sizeof(sockaddr_in);
						auto clientfd = accept(listenSock_, (sockaddr*)&remoteAddr, &addrLen);
						if (clientfd != -1) {
							auto rc = base::Utils::MakeSocketNonBlocking(clientfd);
							acceptedRequest_++;
#if LOG_LOGIN
							char str[20];
							inet_ntop(AF_INET, &remoteAddr.sin_addr, str, 20);
							TRACE_IF(LOG_LOGIN, "%d recv connection from %s, thread id = %d\n",
									(int)acceptedRequest_, str, std::this_thread::get_id());
#endif
							struct kevent ev;
							EV_SET(&ev, clientfd, EVFILT_READ, EV_ADD, 0, 0, 0);
							if (kevent(kq_, &ev, 1, NULL, 0, NULL) < 0) {
								perror("set clientfd kqueue failed\n");
								close(clientfd);
								continue;
							}
						} else {
							if (errno != EAGAIN && errno != EWOULDBLOCK)
								perror("accept error\n");
							break;
						}
					}
				} else {
					if (events_[i].filter == EVFILT_READ) {
						buffer buf(1024);
						size_t len = 1024;
						bool done = false;
						while (true) {
							auto rc = recv(events_[i].ident, buf.data(), len, 0);
							if (rc == -1) {
								if (errno != EAGAIN) {
									perror("read");
									done = true;
								} 
								break;
							} else if (rc == 0) {
								done = true;
								break;
							}
							cmdCenter_.fill(events_[i].ident, buf.data(), rc);
						}
						if (done) {
							close(events_[i].ident);
						}
					} 
				}
			}
		}
	}
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
