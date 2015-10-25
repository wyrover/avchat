// chatserver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <thread>
#include <sstream>
#include <stdio.h>
#include <libconfig.h>
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

const static int kMaxEventsCount = 500;

ChatServer::ChatServer() : cmdCenter_(this)
{
	udpHoleSock_ = -1;
	tcpHoleSock_ = -1;
	listenSock_ = -1;
	kq_ = -1;
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

	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
		return H_SERVER_ERROR;
	rl.rlim_cur = 10240;
	if (setrlimit(RLIMIT_NOFILE, &rl) < 0)
		return H_SERVER_ERROR;
	
	config_t cfg;
	config_setting_t *settings;
	const char *str;
	config_init(&cfg);
	if (!config_read_file(&cfg, "avchat.cfg")) {
		perror("cannot find configure file\n");
		config_destroy(&cfg);
		return H_SERVER_ERROR;
	}
	settings = config_lookup(&cfg, "server");
	if (settings == nullptr) {
		perror("cannot find server config section\n");
		config_destroy(&cfg);
		return H_SERVER_ERROR;
	}
	std::string ip;
	std::string holePort;
	std::string dataPort;
	if (config_setting_lookup_string(settings, "ip", &str)) {
		ip = str;
	} 

	if (config_setting_lookup_string(settings, "holePort", &str)) {
		holePort = str;
	} else {
		perror("server.holePort missing\n");
		config_destroy(&cfg);
		return H_SERVER_ERROR;
	}

	if (config_setting_lookup_string(settings, "dataPort", &str)) {
		dataPort = str;
	} else {
		perror("server.dataPort missing\n");
		config_destroy(&cfg);
		return H_SERVER_ERROR;
	}

	return initSock(ip, dataPort, holePort);
}

HERRCODE ChatServer::initSock(const std::string& ip, const std::string& dataPort,
				const std::string& holePort)
{
	listenSock_ = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSock_ == -1)
		return H_NETWORK_ERROR;

	tcpHoleSock_ = socket(AF_INET, SOCK_DGRAM, 0);
	if (tcpHoleSock_ == -1)
		return H_NETWORK_ERROR;

	udpHoleSock_ = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpHoleSock_ == -1)
		return H_NETWORK_ERROR;

	int rc  = base::Utils::BindSocket(listenSock_, ip, dataPort);
	if (rc != H_OK)
		return rc;

	rc  = base::Utils::BindSocket(tcpHoleSock_, ip, holePort);
	if (rc != H_OK)
		return rc;

	if (base::Utils::MakeSocketNonBlocking(listenSock_) < 0)
		return H_NETWORK_ERROR;

	if (base::Utils::MakeSocketNonBlocking(tcpHoleSock_) < 0)
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

	EV_SET(&ev, tcpHoleSock_, EVFILT_READ, EV_ADD, 0, 0, 0);
	if (kevent(kq_, &ev, 1, NULL, 0, NULL)  < 0) {
		return H_NETWORK_ERROR;
	}

	threads_.push_back(std::thread(&ChatServer::threadFun, this));
	cmdCenter_.start();
	return H_OK;
}

bool ChatServer::setWatchOut(SOCKET sock, bool watch)
{
	struct kevent ev;
	EV_SET(&ev, sock, EVFILT_WRITE,
		   	(watch ? EV_ADD : EV_DELETE), 0, 0, 0);
	if (kevent(kq_, &ev, 1, NULL, 0, NULL) < 0) {
		perror("set clientfd kqueue write failed\n");
		return false;
	}
	return true;
}

HERRCODE ChatServer::queueSendRequest(SOCKET sock, SockStream& stream)
{
	std::lock_guard<std::mutex> guard(sendMutex_);
	auto& buffer = sendBuf_[sock];
	if (!buffer.empty()) {
		buffer.append(stream.getBuf(), stream.getSize());
		return H_OK;
	} else {
		int rc = send(sock, stream.getBuf(), stream.getSize(), 0);
		if (rc > 0) {
			if (rc == stream.getSize()) {
				printf("send request directly done %d\n", stream.getSize());
				return H_OK;
			} else {
				buffer.append(stream.getBuf() + rc, stream.getSize() - rc);
				if (!setWatchOut(sock, true))
					return H_FAILED;
				else
					return H_OK;
			}
		} else {
			if (errno != EAGAIN) {
				//fixme: need handle ENOTBUFS/EPIPE ?
				fprintf(stderr, "send failed errno: %d, %s\n", errno, strerror(errno));
				return H_FAILED;
			}
			buffer.append(stream.getBuf(), stream.getSize());
			if (!setWatchOut(sock, true))
				return H_FAILED;
			else
				return H_OK;
		}
	}
}

bool ChatServer::sendRequest(SOCKET sock, int len)
{
	std::lock_guard<std::mutex> guard(sendMutex_);
	auto& buf = sendBuf_[sock];
	assert(!buf.empty());
	if (len > buf.size())
		len = buf.size();
	int rc = send(sock, buf.data(), len, 0);
	if (rc > 0) {
		if (rc == buf.size()) {
			buf.clear();
			if (!setWatchOut(sock, false))
				return false;
			return true;
		} else {
			auto rptr = buf.data() + rc;
			auto rlen = buf.size() - rc;
			buf.detach();
			buf.assign(rptr, rlen);
			return true;
		}
	} else {
		if (errno != EAGAIN) {
			//fixme: need handle ENOTBUFS/EPIPE ?
			fprintf(stderr, "send failed errno: %d, %s\n", errno, strerror(errno));
			return false;
		}
		return true;
	}
}

int ChatServer::acceptConnections(SOCKET sock)
{
	while (true) {
		sockaddr_in remoteAddr;
		socklen_t addrLen = sizeof(sockaddr_in);
		auto clientfd = accept(sock, (sockaddr*)&remoteAddr, &addrLen);
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
				perror("set clientfd kqueue read failed\n");
				close(clientfd);
				return -1;
			}
		} else {
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				perror("accept error\n");
			return -1;
		}
	}
	return 0;
}

void ChatServer::threadFun()
{
	TRACE_IF(LOG_KQUEUE, "chatserver::run %d\n", std::this_thread::get_id());
	std::vector<struct kevent> events;
	events.resize(kMaxEventsCount);
	struct timespec tmout = { 1, 0 };
	while (!quit_) {
		int eventCount = kevent(kq_, NULL, 0, events.data(), events.size(), &tmout);
		if (eventCount > 0) {
			for (int i = 0; i < eventCount; ++i) {
				printf("kevent %d event\n", eventCount);
				if (events[i].flags & EV_EOF) {
					std::stringstream idStr;
					idStr << std::this_thread::get_id();
					fprintf(stdout, "(%s)kqueue client socket done socket %x\n",
							idStr.str().c_str(), (unsigned int)events[i].ident);
					struct kevent ev;
					EV_SET(&ev, events[i].ident, EVFILT_READ, EV_DELETE, 0, 0, 0);
					if (kevent(kq_, &ev, 1, NULL, 0, NULL) < 0) {
						perror("remove clientfd from kqueue failed\n");
					} 
					close(events[i].ident);
					continue;
				}
				if (events[i].flags & EV_ERROR) {
					perror("kqueue error\n");
					close(events[i].ident);
					continue;
				}

				if (events[i].filter == EVFILT_READ)  {
					if (events[i].ident == listenSock_ || events[i].ident == tcpHoleSock_) {
						acceptConnections(events[i].ident);
					} else {
						puts("handle filt_read");
						size_t len = events[i].data;
						buffer buf(len);
						bool done = false;
						auto rc = recv(events[i].ident, buf.data(), len, 0);
						if (rc == -1) {
							if (errno != EAGAIN) {
								perror("recv error close socket");
								close(events[i].ident);
							} 
							break;
						}
						cmdCenter_.fill(events[i].ident, buf.data(), rc);
					}
				} else if (events[i].filter == EVFILT_WRITE) {
					puts("handle filt_write");
					if (!sendRequest(events[i].ident, events[i].data)) {
						perror("send request failed\n");
						close(events[i].ident);
					}
				}
			}
		}
	}
}

bool ChatServer::quit()
{
	quit_ = true;
	cmdCenter_.quit();
	return true;
}

void ChatServer::wait()
{
	for (auto& thread : threads_){
		thread.join();
	}
}
