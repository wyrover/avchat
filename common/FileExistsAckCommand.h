#pragma once

#include "ChatCommand.h"
#include "SockStream.h"
class FileExistsAckCommand : public ChatCommand
{
public:
	FileExistsAckCommand();
	~FileExistsAckCommand();
	void setUrlList(const std::vector<std::wstring>& urlList, int id);
	std::vector<std::wstring> getUrlList();
	int getId();
	static FileExistsAckCommand* Parse(SockStream* stream);
	virtual void writeTo(SockStream* stream);

private:
	std::vector<std::wstring> urlList_;
	int id_;
};