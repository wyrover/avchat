#pragma once

#include "../common/errcode.h"
#include <string>
class User
{
	friend class Client;
public:
	enum UserStatus {
		kStatus_Invalid,
		kStatus_Online,
		kStatus_Offline,
	};
	User();
	~User();
	HERRCODE login(const std::wstring& email, const std::wstring& password);
	int requestAddFriend(int userId);
	int requestAddGroup(int groupId);
	int removeFriend(int userId);
	int quitGroup(int groupId);
	int logOff();
	int getStatus();

protected:
	int id_;
	std::wstring email_;
	std::wstring username_;
	//std::wstring accessToken_;
	int status_;
};