#include "stdafx.h"
#include "NetConstants.h"
#include "MessageCommand.h"
#include "SockStream.h"

MessageCommand::MessageCommand()
	: ChatCommand(net::kCommandType_Message)
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
	auto size = stream->getInt();
	auto sender = stream->getString();
	auto recver = stream->getString();
	auto timestamp = stream->getInt64();
	auto message = stream->getString();
	cmd->set(sender, recver, message);
	cmd->timeStamp_ = timestamp;
	return cmd;
}

void MessageCommand::writeTo(SockStream* stream)
{
	int size = 0;
	size += stream->writeInt(type_);
	size += stream->writeInt(0); //dummy size
	size += stream->writeString(sender_);
	size += stream->writeString(recver_);
	size += stream->writeInt64(time(NULL));
	size += stream->writeString(message_);
	auto sizePtr = (int*)(stream->getBuf() + 4);
	*sizePtr = size;
}

int64_t MessageCommand::getTimeStamp() const
{
	return timeStamp_;
}
