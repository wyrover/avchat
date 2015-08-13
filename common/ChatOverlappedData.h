#pragma once

#include "buffer.h"

class ChatCommand;

class ChatOverlappedData : public WSAOVERLAPPED
{
public:
	ChatOverlappedData(int type);
	~ChatOverlappedData();
	buffer& getBuf();
	
	void setSocket(SOCKET acceptSock);
	SOCKET getSocket();
	int getNetType() const;
	void setNetType(int type);
	void setProp(int value);
	int getProp();

private:
	int netType_;
	buffer recvBuf_;
	SOCKET sock_;
	int prop_;
};