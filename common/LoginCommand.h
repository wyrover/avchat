#pragma once

#include "ChatCommand.h"

class SockStream;

class LoginCommand : public ChatCommand
{
public:
	LoginCommand();
	virtual ~LoginCommand();
	void set(const std::wstring& username, const std::wstring& password);
	std::wstring getUsername() const;
	std::wstring getPassword() const;

	static LoginCommand* Parse(SockStream* stream);
	virtual void writeTo(SockStream* stream);

private:
	std::wstring username_;
	std::wstring password_;
};