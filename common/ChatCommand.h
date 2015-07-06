#pragma once

class SockStream;

// login: username + password
// client-msg: destination(broadcast/username) + message
// server-msg: destination(broadcast/username) + message
class ChatCommand
{
public:
	enum Action {
		kAction_Login,
		kAction_ClientMsg,
		kAction_ServerMsg,
	};
public:
	ChatCommand(int type);
	~ChatCommand();
	virtual void writeTo(SockStream* buff) = 0;
	
protected:
	int type_;
};