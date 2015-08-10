#include "stdafx.h"
#include "FileExistsAckCommand.h"
#include "NetConstants.h"
#include "SockStream.h"

FileExistsAckCommand::FileExistsAckCommand()
	: ChatCommand(net::kCommandType_FileExistsAck)
{
}

FileExistsAckCommand::~FileExistsAckCommand()
{
}

FileExistsAckCommand* FileExistsAckCommand::Parse(SockStream* stream)
{
	auto cmd = new FileExistsAckCommand();
	auto size = stream->getInt();
	auto id = stream->getInt();
	auto fileList = stream->getStringVec();
	cmd->setUrlList(fileList, id);
	return cmd;
}

void FileExistsAckCommand::writeTo(SockStream* stream)
{
	int size = 0;
	size += stream->writeInt(type_);
	size += stream->writeInt(0); //dummy size
	size += stream->writeInt(id_);
	size += stream->writeStringVec(urlList_);
	auto sizePtr = (int*)(stream->getBuf() + 4);
	*sizePtr = size;
}

void FileExistsAckCommand::setUrlList(const std::vector<std::wstring>& urlList, int id)
{
	urlList_ = urlList;
	id_ = id;
}

std::vector<std::wstring> FileExistsAckCommand::getUrlList()
{
	return urlList_;
}

int FileExistsAckCommand::getId()
{
	return id_;
}
