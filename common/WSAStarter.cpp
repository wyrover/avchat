#include "stdafx.h"
#include "WSAStarter.h"


WSAStarter::WSAStarter()
{
}

WSAStarter::~WSAStarter()
{
	WSACleanup();
}

bool WSAStarter::init()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
		return false;
	return true;
}