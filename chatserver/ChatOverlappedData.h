#pragma once

class ChatOverlappedData : public WSAOVERLAPPED
{
public:
	enum Type {
		kType_Accept,
		kType_AcceptAck,
		kType_Send,
		kType_Recv,
	};
public:
	ChatOverlappedData(int type);
	~ChatOverlappedData();
	void setAcceptBuffer(char* acceptBuff);
	char* getAccpetBuffer();
	void setAcceptSock(SOCKET acceptSock);
	SOCKET getAcceptSock();
	int getType();
	void setType(int type);

private:
	int type_;
	char* acceptBuff_;
	SOCKET acceptSock_;
};