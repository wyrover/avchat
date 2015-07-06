#include "stdafx.h"
#include "ChatCommand.h"

ChatCommand::ChatCommand(int type)
	: type_(type)
{

}

ChatCommand::~ChatCommand()
{
}

int ChatCommand::getType() const
{
	return type_;
}
