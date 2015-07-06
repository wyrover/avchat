#pragma once

class ChatCommand;

class ChatOverlappedData : public WSAOVERLAPPED
{
public:
	enum Type {
		kType_Accept,
		kType_LoginAck,
		kType_Send,
		kType_Recv,
	};
public:
	ChatOverlappedData(int type);
	~ChatOverlappedData();
	buffer& getBuf();
	buffer& getCmdBuf();
	int& getCmdNeedSize();
	
	void setSocket(SOCKET acceptSock);
	SOCKET getSocket();
	int getType();
	void setType(int type);

private:
	int type_;
	buffer recvBuf_;
	buffer cmdBuf_;
	SOCKET sock_;
	int cmdNeedSize_;
};