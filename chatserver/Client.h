#pragma once

#include "User.h"

class Client
{
public:
	Client(const User& user, SOCKET socket);
	~Client();
	SOCKET getSocket();
	std::wstring getUsername() const;
	std::wstring getEmail() const;
	void logout();

private:
	User user_;
	SOCKET socket_;
};