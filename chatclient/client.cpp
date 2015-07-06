#include "stdafx.h"
#include "ChatClient.h"

int _tmain(int argc, _TCHAR* argv[])
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	int count = 10000;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	assert(err == 0);

	int port = std::stoi(argv[2]);
	std::vector<std::thread> threads;

	for (int i = 0; i < 10; ++i) {
		threads.push_back(std::thread([] {
			for (int i = 0; i < 1024; ++i) {
				ChatClient* client = new ChatClient(L"127.0.0.1", 2333);
				client->connect(L"keke", L"haha");
			}
		}));
	}

	for (auto& thread : threads) {
		thread.join();
	}
	WSACleanup();
	return 0;
}