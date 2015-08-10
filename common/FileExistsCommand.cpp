#include "stdafx.h"
#include "FileExistsCommand.h"
#include "NetConstants.h"
#include "SockStream.h"

FileExistsCommand::FileExistsCommand()
	: ChatCommand(net::kCommandType_FileExists)
{
}

FileExistsCommand::~FileExistsCommand()
{
}

FileExistsCommand* FileExistsCommand::Parse(SockStream* stream)
{
	auto cmd = new FileExistsCommand();
	auto size = stream->getInt();
	auto id = stream->getInt();
	auto hashList = stream->getStringVec();
	cmd->setHashList(hashList, id);
	return cmd;
}

void FileExistsCommand::writeTo(SockStream* stream)
{
	int size = 0;
	size += stream->writeInt(type_);
	size += stream->writeInt(0); //dummy size
	size += stream->writeInt(id_);
	size += stream->writeStringVec(hashList_);
	auto sizePtr = (int*)(stream->getBuf() + 4);
	*sizePtr = size;
}

void FileExistsCommand::setHashList(const std::vector<std::wstring>& hashList, int id)
{
	hashList_ = hashList;
	id_ = id;
}

std::vector<std::wstring> FileExistsCommand::getHashList_()
{
	return hashList_;
}

int FileExistsCommand::getId()
{
	return id_;
}
