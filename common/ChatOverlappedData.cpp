#include "stdafx.h"
#include "ChatOverlappedData.h"

ChatOverlappedData::ChatOverlappedData(int type)
	: netType_(type), recvBuf_(1024)
{
	WSAOVERLAPPED* ol = (WSAOVERLAPPED*)this;
	memset(ol, 0, sizeof(WSAOVERLAPPED));
	cmdNeedSize_ = -1;
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

buffer& ChatOverlappedData::getCmdBuf()
{
	return cmdBuf_;
}

int& ChatOverlappedData::getCmdNeedSize()
{
	return cmdNeedSize_;
}

int ChatOverlappedData::getCommandType() const
{
	return commandType_;
}

void ChatOverlappedData::setCommandType(int type)
{
	commandType_ = type;
}

void ChatOverlappedData::setMessage(const std::wstring& message)
{
	message_ = message;
}

std::wstring ChatOverlappedData::getMessage()
{
	return message_;
}
