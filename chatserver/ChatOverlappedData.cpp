#include "stdafx.h"
#include "ChatOverlappedData.h"

ChatOverlappedData::ChatOverlappedData(int type)
	: type_(type), recvBuf_(1024)
{
	WSAOVERLAPPED* ol = (WSAOVERLAPPED*)this;
	memset(ol, 0, sizeof(WSAOVERLAPPED));
	cmdNeedSize_ = -1;
}

ChatOverlappedData::~ChatOverlappedData()
{
}

int ChatOverlappedData::getType()
{
	return type_;
}

void ChatOverlappedData::setSocket(SOCKET acceptSock)
{
	sock_ = acceptSock;
}

SOCKET ChatOverlappedData::getSocket()
{
	return sock_;
}

void ChatOverlappedData::setType(int type)
{
	type_ = type;
}

buffer& ChatOverlappedData::getBuf()
{
	return recvBuf_;
}

buffer& ChatOverlappedData::getCmdBuf()
{
	return cmdBuf_;
}

int& ChatOverlappedData::getCmdNeedSize()
{
	return cmdNeedSize_;
}
