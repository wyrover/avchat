#pragma once

#include "ChatCommand.h"

class FileExistsCommand : public ChatCommand
{
public:
	FileExistsCommand();
	~FileExistsCommand();
	void setHashList(const std::vector<std::wstring>& hashList, int id);
	std::vector<std::wstring> getHashList_();
	int getId();
	static FileExistsCommand* Parse(SockStream* stream);
	virtual void writeTo(SockStream* stream);

private:
	std::vector<std::wstring> hashList_;
	int id_;
};