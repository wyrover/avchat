#include <cstdio>

#include "../common/errcode.h"
#include "unixserver.h"

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
