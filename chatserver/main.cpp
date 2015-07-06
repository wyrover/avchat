#include "stdafx.h"
#include "ChatServer.h"


int _tmain(int argc, _TCHAR* argv[])
{
	ChatServer server;
	server.init();
	server.run();
	return 0;
}