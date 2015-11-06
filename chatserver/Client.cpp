#include "stdafx.h"
#include "Client.h"

Client::Client(const User& user, SOCKET socket)
: user_(user), socket_(socket)
{

}

Client::~Client()
{
}

SOCKET Client::getSocket()
{
	return socket_;
}

std::u16string Client::getUsername() const
{
	return user_.username_;
}

std::u16string Client::getEmail() const
{
	return user_.email_;
}

void Client::logout()
{
	user_.logout();
	close(socket_);
}

void Client::setAddr(const sockaddr_in& localAddr, const sockaddr_in& publicAddr)
{
	localAddr_ = localAddr;
	publicAddr_ = publicAddr;
}

sockaddr_in Client::getLocalAddr()
{
	return localAddr_;
}

sockaddr_in Client::getPublicAddr()
{
	return publicAddr_;
}

std::u16string Client::getAuthKey() const
{
	return user_.getAuthKey();
}
