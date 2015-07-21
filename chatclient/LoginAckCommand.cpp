#include "stdafx.h"
#include "../common/NetConstants.h"
#include "../common/SockStream.h"
#include "LoginAckCommand.h"

LoginAckCommand::LoginAckCommand()
	: ChatCommand(net::kCommandType_LoginAck)
{
}


LoginAckCommand::~LoginAckCommand()
{
}

int LoginAckCommand::getResult()
{
	return result_;
}

LoginAckCommand* LoginAckCommand::Parse(SockStream* stream)
{
	stream->getInt(); // size
	auto result = stream->getInt();
	LoginAckCommand* msg = new LoginAckCommand();
	msg->result_ = result;
	return msg;
}

void LoginAckCommand::writeTo(SockStream* buff)
{
	assert(false);
}
