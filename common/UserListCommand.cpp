#include "stdafx.h"
#include "NetConstants.h"
#include "SockStream.h"
#include "UserListCommand.h"


UserListCommand::UserListCommand()
	: ChatCommand(net::kCommandType_UserList)
{
}


UserListCommand::~UserListCommand()
{
}

void UserListCommand::setUserList(const std::vector<std::wstring>& userList)
{
	userList_ = userList;
}

std::vector<std::wstring> UserListCommand::getUserList() const
{
	return userList_;
}

UserListCommand* UserListCommand::Parse(SockStream* stream)
{
	UserListCommand* cmd = new UserListCommand();
	int size = stream->getInt();
	int count = stream->getInt();
	assert(count > 0);
	std::lock_guard<std::recursive_mutex> guard(cmd->userMutex_);
	cmd->userList_.clear();
	while (count--) {
		auto username = stream->getString();
		cmd->userList_.push_back(username);
	}
	return cmd;
}

void UserListCommand::writeTo(SockStream* buf)
{
	assert(false);
}
