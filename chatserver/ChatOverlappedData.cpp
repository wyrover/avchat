#include "stdafx.h"
#include "ChatOverlappedData.h"

ChatOverlappedData::ChatOverlappedData(int type)
	: type_(type)
{
	WSAOVERLAPPED* ol = (WSAOVERLAPPED*)this;
	memset(ol, 0, sizeof(WSAOVERLAPPED));
}

ChatOverlappedData::~ChatOverlappedData()
{
}

void ChatOverlappedData::setAcceptBuffer(char* acceptBuff)
{
	acceptBuff_ = acceptBuff;
}

char* ChatOverlappedData::getAccpetBuffer()
{
	return acceptBuff_;
}

int ChatOverlappedData::getType()
{
	return type_;
}

void ChatOverlappedData::setAcceptSock(SOCKET acceptSock)
{
	acceptSock_ = acceptSock;
}

SOCKET ChatOverlappedData::getAcceptSock()
{
	return acceptSock_;
}

void ChatOverlappedData::setType(int type)
{
	type_ = type;
}
