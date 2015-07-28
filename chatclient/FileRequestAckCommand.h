#pragma once

#include "../common/ChatCommand.h"

class FileRequestAckCommand : public ChatCommand
{
public:
	FileRequestAckCommand();
	~FileRequestAckCommand();
	int getResult();
	static FileRequestAckCommand* Parse(SockStream* stream);
	void writeTo(SockStream* ss);

private:
	int result_;
};