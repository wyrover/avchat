#pragma once

#include "../common/ChatCommand.h"

class FileRequestAckCommand : public ChatCommand
{
public:
	FileRequestAckCommand();
	~FileRequestAckCommand();
	void init(const std::wstring& local, const std::wstring& remote, time_t timestamp, bool allow);
	time_t getTimeStamp();
	std::wstring getLocal();
	std::wstring getRemote();
	bool isAllow();
	static FileRequestAckCommand* Parse(SockStream* stream);
	void writeTo(SockStream* ss);

private:
	std::wstring local_;
	std::wstring remote_;
	time_t timestamp_;
	bool allow_;
};