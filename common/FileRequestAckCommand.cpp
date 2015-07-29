#include "stdafx.h"
#include "NetConstants.h"
#include "SockStream.h"
#include "FileRequestAckCommand.h"

FileRequestAckCommand::FileRequestAckCommand()
	: ChatCommand(net::kCommandType_FileRequestAck)
{

}

FileRequestAckCommand::~FileRequestAckCommand()
{

}

FileRequestAckCommand* FileRequestAckCommand::Parse(SockStream* ss)
{
	auto size = ss->getInt();
	auto remote = ss->getString();
	auto local = ss->getString();
	auto timestamp = ss->getInt64();
	auto allow = ss->getBool();
	FileRequestAckCommand* cmd = new FileRequestAckCommand;
	cmd->init(local, remote, timestamp, allow);
	return cmd;
}

void FileRequestAckCommand::writeTo(SockStream* ss)
{
	int size = 0;
	size += ss->writeInt(net::kCommandType_FileRequestAck);
	size += ss->writeInt(0); //dummy size
	size += ss->writeString(local_);
	size += ss->writeString(remote_);
	size += ss->writeInt64(timestamp_);
	size += ss->writeBool(allow_);
	auto sizePtr = (int*)(ss->getBuf() + 4);
	*sizePtr = size;
}

void FileRequestAckCommand::init(const std::wstring& local, const std::wstring& remote, time_t timestamp, bool allow)
{
	local_ = local;
	remote_ = remote;
	timestamp_ = timestamp;
	allow_ = allow;
}

time_t FileRequestAckCommand::getTimeStamp()
{
	return timestamp_;
}

std::wstring FileRequestAckCommand::getLocal()
{
	return local_;
}

std::wstring FileRequestAckCommand::getRemote()
{
	return remote_;
}

bool FileRequestAckCommand::isAllow()
{
	return allow_;
}