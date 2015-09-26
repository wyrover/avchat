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
	HERRCODE login(int authType, const std::u16string& email, const std::u16string& password);
	HERRCODE logout();

	int requestAddFriend(int userId);
	int requestAddGroup(int groupId);
	int removeFriend(int userId);
	int quitGroup(int groupId);
	int getStatus();
	std::u16string getAuthKey();

	protected:
	int id_;
	std::u16string email_;
	std::u16string username_;
	std::u16string authKey_;
	int status_;
};
