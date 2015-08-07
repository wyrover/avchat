#include "stdafx.h"
#include "ChatServer.h"
#include "Utils.h"
#include "SimpleTest.h"
#include "../common/errcode.h"

int _tmain(int argc, _TCHAR* argv[])
{
	ChatServer server;
	auto hr = server.init();
	if (hr != H_OK) {
		printf("server init error\n");
		return -1;
	}
	server.run();
	return 0;
}