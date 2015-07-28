#include "stdafx.h"
#include "../common/NetConstants.h"
#include "FileRequestAckCommand.h"

FileRequestAckCommand::FileRequestAckCommand()
	: ChatCommand(net::kCommandType_FileRequestAck)
{

}

FileRequestAckCommand::~FileRequestAckCommand()
{

}

int FileRequestAckCommand::getResult()
{
	return 0;
}

FileRequestAckCommand* FileRequestAckCommand::Parse(SockStream* stream)
{
	return nullptr;
}

void FileRequestAckCommand::writeTo(SockStream* ss)
{

}
