#pragma once

#include "ChatCommand.h"
class SockStream;

class FileRequestCommand : public ChatCommand
{
public:
	FileRequestCommand();
	~FileRequestCommand();
	void init(const std::wstring& sender, const std::wstring& recver,
		const std::wstring& filePath, bool isFolder, int64_t fileSize, time_t timestamp);
	std::wstring getSender();
	std::wstring getRecver();
	std::wstring getFileName();
	bool isFolder();
	int64_t getFileSize();
	static FileRequestCommand* parse(SockStream* ss);
	void writeTo(SockStream* ss);

private:
	std::wstring filePath_;
	bool isFolder_;
	int64_t fileSize_;
	std::wstring sender_;
	std::wstring recver_;
	time_t timestamp_;
};