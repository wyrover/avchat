#include "stdafx.h"
#include "unixserver.h"
#include "Utils.h"
#include "../common/errcode.h"

int main(int argc, char** argv)
{
	ChatServer server;
	auto hr = server.start();
	if (hr != H_OK) {
		perror("server init error\n");
		return -1;
	}
	server.wait();
	return 0;
}
