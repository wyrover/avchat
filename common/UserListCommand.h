#pragma once

#include "ChatCommand.h"
class SockStream;
class UserListCommand : public ChatCommand
{
public:
	UserListCommand();
	virtual ~UserListCommand();
	void setUserList(const std::vector<std::wstring>& userList);
	std::vector<std::wstring> getUserList() const;
	static UserListCommand* Parse(SockStream* stream);
	virtual void writeTo(SockStream* buf);

private:
	std::vector<std::wstring> userList_;
	std::mutex userMutex_;
};

