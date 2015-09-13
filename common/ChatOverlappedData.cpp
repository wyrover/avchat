#include "stdafx.h"
#include "ChatOverlappedData.h"

ChatOverlappedData::ChatOverlappedData(int type)
	: netType_(type), recvBuf_(1024)
{
	WSAOVERLAPPED* ol = (WSAOVERLAPPED*)this;
	memset(ol, 0, sizeof(WSAOVERLAPPED));
}

ChatOverlappedData::~ChatOverlappedData()
{
}

int ChatOverlappedData::getNetType() const
{
	return netType_;
}

void ChatOverlappedData::setSocket(SOCKET acceptSock)
{
	sock_ = acceptSock;
}

SOCKET ChatOverlappedData::getSocket()
{
	return sock_;
}

void ChatOverlappedData::setNetType(int type)
{
	netType_ = type;
}

buffer& ChatOverlappedData::getBuf()
{
	return recvBuf_;
}

void ChatOverlappedData::setProp(int value)
{
	prop_ = value;
}

int ChatOverlappedData::getProp()
{
	return prop_;
}

sockaddr_in* ChatOverlappedData::getAddr()
{
	return &addr_;
}

int * ChatOverlappedData::getAddrLen()
{
	return &addrLen_;
}

int * ChatOverlappedData::getBytesSent()
{
	return &bytesSent_;
}
