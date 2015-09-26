#pragma once

#include "User.h"

class Client
{
public:
	Client(const User& user, SOCKET socket);
	~Client();
	SOCKET getSocket();
	std::u16string getUsername() const;
	std::u16string getEmail() const;
	void logout();

private:
	User user_;
	SOCKET socket_;
};
