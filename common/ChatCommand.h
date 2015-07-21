#pragma once

class SockStream;

// login: username + password
// client-msg: destination(broadcast/username) + message
// server-msg: destination(broadcast/username) + message
class ChatCommand
{
public:
	ChatCommand(int type);
	virtual ~ChatCommand();
	virtual void writeTo(SockStream* buff) = 0;
	int getType() const;
	
protected:
	int type_;
};