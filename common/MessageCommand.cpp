#include "stdafx.h"
#include "MessageCommand.h"
#include "SockStream.h"

MessageCommand::MessageCommand()
	: ChatCommand(ChatCommand::kAction_ClientMsg)
{
}

MessageCommand::~MessageCommand()
{
}

void MessageCommand::set(const std::wstring& sender, const std::wstring& recv, const std::wstring& message)
{
	sender_ = sender;
	recver_ = recv;
	message_ = message;
}

std::wstring MessageCommand::getSender() const
{
	return sender_;
}

std::wstring MessageCommand::getReceiver() const
{
	return recver_;
}

std::wstring MessageCommand::getMessage() const
{
	return message_;
}

MessageCommand* MessageCommand::Parse(SockStream* stream)
{
	MessageCommand* cmd = new MessageCommand();
	auto sender = stream->getString();
	auto recver = stream->getString();
	auto message = stream->getString();
	cmd->set(sender, recver, message);
	return cmd;
}

void MessageCommand::writeTo(SockStream* stream)
{
	int size = 0;
	size += stream->writeInt(type_);
	int* psize = (int*)stream->getCurrPtr();
	size += stream->writeString(sender_);
	size += stream->writeString(recver_);
	size += stream->writeString(message_);
	*psize = size;
}