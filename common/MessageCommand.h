#pragma once

#include "ChatCommand.h"

class MessageCommand : public ChatCommand
{
public:
	MessageCommand();
	~MessageCommand();
	void set(const std::wstring& sender, const std::wstring& recv, const std::wstring& message, time_t timestamp);
	std::wstring getSender() const;
	std::wstring getReceiver() const;
	std::wstring getMessage() const;
	int64_t getTimeStamp() const;
	static MessageCommand* Parse(SockStream* stream);
	virtual void writeTo(SockStream* stream);

private:
	std::wstring sender_;
	std::wstring recver_;
	std::wstring message_;
	time_t timeStamp_;
};