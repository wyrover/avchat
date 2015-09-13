#include "stdafx.h"
#include "ChatServer.h"
#include "Utils.h"
#include "SimpleTest.h"
#include "../common/errcode.h"
#include "../common/WSAStarter.h"

int _tmain(int argc, _TCHAR* argv[])
{
	WSAStarter starter;
	if (!starter.init())
		return -1;
	ChatServer server;
	auto hr = server.start();
	if (hr != H_OK) {
		printf("server init error\n");
		return -1;
	}
	server.wait();
	return 0;
}