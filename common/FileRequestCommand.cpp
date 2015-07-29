#include "stdafx.h"
#include "SockStream.h"
#include "FileRequestCommand.h"
#include "NetConstants.h"
#include "FileUtils.h"
#include <Shlwapi.h>

FileRequestCommand::FileRequestCommand()
	: ChatCommand(net::kCommandType_FileRequest)
{
}

FileRequestCommand::~FileRequestCommand()
{
}

std::wstring FileRequestCommand::getFileName()
{
	return filePath_;
}

bool FileRequestCommand::isFolder()
{
	return isFolder_;
}

int64_t FileRequestCommand::getFileSize()
{
	return fileSize_;
}

FileRequestCommand* FileRequestCommand::parse(SockStream* ss)
{
	auto size = ss->getInt();
	auto sender = ss->getString();
	auto recver = ss->getString();
	auto timestamp = ss->getInt64();
	auto filename = ss->getString();
	auto isFolder = ss->getBool();
	auto fileSize = ss->getInt64();
	auto cmd = new FileRequestCommand();
	cmd->init(sender, recver, filename, isFolder, fileSize, timestamp);
	return cmd;
}

void FileRequestCommand::writeTo(SockStream* ss)
{
	int size = 0;
	size += ss->writeInt(net::kCommandType_FileRequest);
	size += ss->writeInt(0); //dummy size
	size += ss->writeString(sender_);
	size += ss->writeString(recver_);
	size += ss->writeInt64(timestamp_);
	size += ss->writeString(::PathFindFileName(filePath_.c_str()));
	size += ss->writeBool(FileUtils::DirExists(filePath_.c_str()));
	size += ss->writeInt64(fileSize_);
	auto sizePtr = (int*)(ss->getBuf() + 4);
	*sizePtr = size;
}

void FileRequestCommand::init(const std::wstring& sender, const std::wstring& recver,
	const std::wstring& filePath, bool isFolder, int64_t fileSize, time_t timestamp)
{
	sender_ = sender;
	recver_ = recver;
	filePath_ = filePath;
	isFolder_ = isFolder;
	fileSize_ = fileSize;
	timestamp_ = timestamp;
}

std::wstring FileRequestCommand::getSender()
{
	return sender_;
}

std::wstring FileRequestCommand::getRecver()
{
	return recver_;
}