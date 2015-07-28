#pragma once

#include "../common/ChatCommand.h"

class LoginAckCommand : public ChatCommand
{
public:
	LoginAckCommand();
	~LoginAckCommand();
	int getResult();
	static LoginAckCommand* Parse(SockStream* stream);
	void writeTo(SockStream* ss);
private:
	int result_;
};

