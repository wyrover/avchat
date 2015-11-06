#pragma once

#include "User.h"
#include <arpa/inet.h>
#include <netinet/in.h>
class Client
{
public:
	Client(const User& user, SOCKET socket);
	~Client();
	SOCKET getSocket();
	std::u16string getUsername() const;
	std::u16string getEmail() const;
	std::u16string getAuthKey() const;
	void logout();
	void setAddr(const sockaddr_in& localAddr, const sockaddr_in& publicAddr);
	sockaddr_in getLocalAddr();
	sockaddr_in getPublicAddr();

private:
	User user_;
	SOCKET socket_;
	sockaddr_in publicAddr_;
	sockaddr_in localAddr_;
};
