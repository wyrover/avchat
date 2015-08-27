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
	HERRCODE login(int authType, const std::wstring& email, const std::wstring& password);
	HERRCODE logout();

	int requestAddFriend(int userId);
	int requestAddGroup(int groupId);
	int removeFriend(int userId);
	int quitGroup(int groupId);
	int getStatus();
	std::wstring getAuthKey();

protected:
	int id_;
	std::wstring email_;
	std::wstring username_;
	std::wstring authKey_;
	int status_;
};