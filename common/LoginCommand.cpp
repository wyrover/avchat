#include "stdafx.h"
#include "LoginCommand.h"
#include "SockStream.h"
#include "NetConstants.h"

LoginCommand::LoginCommand()
	: ChatCommand(net::kCommandType_Login)
{
}

LoginCommand::~LoginCommand()
{
}

std::wstring LoginCommand::getUsername() const
{
	return username_;
}

std::wstring LoginCommand::getPassword() const
{
	return password_;
}

void LoginCommand::set(const std::wstring& username, const std::wstring& password)
{
	username_ = username;
	password_ = password;
}

LoginCommand* LoginCommand::Parse(SockStream* stream)
{
	stream->getInt();
	auto username = stream->getString();
	auto password = stream->getString();
	LoginCommand* msg = new LoginCommand();
	msg->set(username, password);
	return msg;
}

void LoginCommand::writeTo(SockStream* stream)
{
	int sum = 0;
	int str_size = 0;
	sum += stream->writeInt(type_);
	auto s= stream->writeInt(0);
	sum += s; str_size += s;
	s = stream->writeString(username_);
	sum += s; str_size += s;
	s = stream->writeString(password_);
	sum += s; str_size += s;
	
	int* psize = (int*)(stream->getCurrPtr() - str_size);
	*psize = sum;
}