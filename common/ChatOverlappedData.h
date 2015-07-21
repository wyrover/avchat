#pragma once

#include "buffer.h"

class ChatCommand;

class ChatOverlappedData : public WSAOVERLAPPED
{
public:
	ChatOverlappedData(int type);
	~ChatOverlappedData();
	buffer& getBuf();
	buffer& getCmdBuf();
	int& getCmdNeedSize();
	
	void setSocket(SOCKET acceptSock);
	SOCKET getSocket();
	int getNetType() const;
	void setNetType(int type);
	int getCommandType() const;
	void setCommandType(int type);

private:
	int netType_;
	int commandType_;
	buffer recvBuf_;
	buffer cmdBuf_;
	SOCKET sock_;
	int cmdNeedSize_;
};