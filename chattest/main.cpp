#include <unistd.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>

#include <assert.h>
#include <time.h>
#include <memory>
#include <vector>
#include <thread>
#include <iostream>
#include "../chatclient/ChatClient.h"
#include "../common/Utils.h"
#include "../common/StringUtils.h"
#include "ChatClientController.h"

#define SERVER_ADDRESS u"192.168.134.125"

volatile bool quit = false;

namespace test1
{
	void test(int kq)
	{
		auto controller = new ChatClientController();
		std::unique_ptr<avc::ChatClient> client(new avc::ChatClient());
		if (client->init(SERVER_ADDRESS, 2333, kq) != H_OK) {
			syslog(LOG_ERR, "init error\n");
			return;
		}
		client->setController(controller);
		if (client->login(u"sss@sss.com", u"kekeke") != H_OK) {
			syslog(LOG_ERR, "login error\n");
			return;
		}
		while (!quit) {
			if (client->sendMessage(u"all", u"how are you", time(NULL)) != H_OK) {
				syslog(LOG_ERR, "sendmessage error\n");
				return;
			}
		}
	}

	int test1()
	{
		int kq = kqueue();
		if (kq == -1) {
			syslog(LOG_ERR, "create kqueue failed\n");
			return -1;
		}
		std::vector<std::thread> threads;
		for (int i = 0; i < 500; ++i) {
			threads.push_back(std::thread(&test, kq));
		}
		for (auto& thread : threads) {
			thread.join();
		}
		return 0;
	}
}

namespace test2
{
	int test(int kq, ChatClientController* controller)
	{
		std::vector<std::shared_ptr<avc::ChatClient>> clients;
		for (int i = 0; i < 10; ++i) {
			std::shared_ptr<avc::ChatClient> client(new avc::ChatClient());
			if (client->init(SERVER_ADDRESS, 2333, kq) != H_OK) {
				syslog(LOG_ERR, "init error\n");
				return -1;
			}
			client->setController(controller);
			if (client->login(u"sss@sss.com", u"kekeke") != H_OK) {
				syslog(LOG_ERR, "login error\n");
				return -1;
			}
			clients.push_back(client);
		}

		while (!quit) {
			for (auto& client : clients) {
				syslog(LOG_DEBUG, "chatclient try to send message\n");
				if (client->sendMessage(u"all", u"how are you", time(NULL)) != H_OK) {
					syslog(LOG_ERR, "sendmessage error\n");
					return - 1;
				}
			}
		}
		return 0;
	}

	int test2()
	{
		auto controller = new ChatClientController();
		int kq = kqueue();
		if (kq == -1) {
			syslog(LOG_ERR, "create kqueue failed\n");
			return -1;
		}

		std::vector<std::thread> threads;
		for (int i = 0; i < 50; ++i) {
			threads.push_back(std::thread(test, kq, controller));
		}
		for (auto& thread : threads) {
			thread.join();
		}
		return 0;
	}
}

namespace rawtest
{
	int test(int kq)
	{
		int sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock_ == -1) {
			syslog(LOG_ERR, "cannot create socket\n");
			return H_NETWORK_ERROR;
		}
		syslog(LOG_DEBUG, "created socket %d\n", sock_);
		if (base::Utils::MakeSocketNonBlocking(sock_) < 0) {
			syslog(LOG_ERR, "cannot make socket nonblocking\n");
			return H_NETWORK_ERROR;
		}
		sockaddr_in addr = { 0 };
		addr.sin_port = htons(2333);
		addr.sin_family = AF_INET;
		in_addr iaddr;
		auto uServerAddr = su::u16to8(SERVER_ADDRESS);
		if (inet_pton(AF_INET, uServerAddr.c_str(), &iaddr) != 1) {
			syslog(LOG_ERR, "network address translate failed\n");
			return H_NETWORK_ERROR;
		}
		addr.sin_addr = iaddr;
		// is there any bug here? attach but connect returned?
		auto err = connect(sock_, (const sockaddr*)&addr, sizeof(sockaddr_in));
		if (err == 0) {
			struct kevent ev;
			EV_SET(&ev, sock_, EVFILT_READ, EV_ADD, 0, 0, 0);
			if (kevent(kq, &ev, 1, NULL, 0, 0) < 0) {
				syslog(LOG_ERR, "kqeueue change client socket failed\n");
				close(sock_);
				sock_ = -1;
				return H_NETWORK_ERROR;
			}
			syslog(LOG_DEBUG, "kqueue add socket %d filt-read succeeded\n", sock_);
		} else if (err == -1 && errno != EINPROGRESS) {
			syslog(LOG_ERR, "connect errrno %d, %s\n", errno, strerror(errno));
			return H_NETWORK_ERROR;
		} else {
			if (kq == -1) {
				kq = kqueue();
				if (kq == -1) {
					syslog(LOG_ERR, "kqueue create failed\n");
					return H_SERVER_ERROR;
				}
			}
			struct kevent ev;
			EV_SET(&ev, sock_, EVFILT_WRITE, EV_ADD, 0, 0, 0);
			if (kevent(kq, &ev, 1, NULL, 0, 0) < 0) {
				syslog(LOG_ERR, "kqueue %d add socket %d filt-write failed errno: %d, %s\n", 
						kq, sock_, errno, strerror(errno));
				close(sock_);
				sock_ = -1;
				return H_NETWORK_ERROR;
			} else {
				syslog(LOG_DEBUG, "kqueue %d add socket %d filt-write succeeded\n", 
						kq, sock_);
			}
		}
		return H_OK;
	}
	int rawtest()
	{
		int kq = kqueue();
		if (kq == -1) {
			syslog(LOG_ERR, "create kqueue failed\n");
			return -1;
		}
		for (int i = 0; i < 100; ++i) {
			test(kq);
		}
		return 0;
	}
		
}

int simpletest()
{
	auto controller = new ChatClientController();
	int kq = kqueue();
	std::unique_ptr<avc::ChatClient> client(new avc::ChatClient());
	if (client->init(SERVER_ADDRESS, 2333, kq) != H_OK) {
		syslog(LOG_ERR, "init error\n");
		return -1;
	}
	client->setController(controller);
	if (client->login(u"sss@sss.com", u"kekeke") != H_OK) {
		syslog(LOG_ERR, "login error\n");
		return -1;
	}
	if (client->sendMessage(u"all", u"how are you", time(NULL)) != H_OK) {
		syslog(LOG_ERR, "sendmessage error\n");
		return -1;
	}
	return -1;
}


int main(int argc, char *argv[])
{
	openlog("chattest", LOG_PID | LOG_CONS | LOG_NDELAY, LOG_USER);

	struct rlimit limit;
	getrlimit(RLIMIT_NOFILE, &limit);
	printf("before %llu, %llu\n", limit.rlim_cur, limit.rlim_max);

	limit.rlim_cur = 10240;
	setrlimit(RLIMIT_NOFILE, &limit);

	getrlimit(RLIMIT_NOFILE, &limit);
	printf("after %llu, %llu\n", limit.rlim_cur, limit.rlim_max);
	syslog(LOG_ERR, "kekekekekek");
	//int rc = test1::test1();
	//int rc = rawtest::rawtest();
	int rc = test2::test2();
	//int rc = simpletest();
	closelog();
	return rc;
}

