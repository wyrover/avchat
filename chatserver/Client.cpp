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

std::wstring Client::getUsername() const
{
	return user_.username_;
}

std::wstring Client::getEmail() const
{
	return user_.email_;
}

void Client::logout()
{
	user_.logout();
	closesocket(socket_);
}
