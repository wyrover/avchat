#include "stdafx.h"

#include <boost/regex.hpp>
#include "User.h"
#include "Utils.h"
#include "ServerContext.h"
#include "DBContext.h"
#include "../common/errcode.h"

std::unique_ptr<sql::PreparedStatement> User::statusStmt_;
std::unique_ptr<sql::PreparedStatement> User::loginStmt_;

User::User()
{
}

User::~User()
{
}

int User::login(const std::wstring& email, const std::wstring& password)
{
	int hr = createPreparedStatement();
	if (hr != H_OK)
		return hr;
	try {
		auto utf8email = Utils::Utf16ToUtf8String(email);
		loginStmt_->setString(1, utf8email);
		std::unique_ptr<sql::ResultSet> res(loginStmt_->executeQuery());
		if (res->rowsCount() != 1) {
			return H_AUTH_FAILED;
		}
		while (res->next()) {
			auto password_hash = res->getString("password_hash");
			if (!Utils::ValidatePasswordHash(Utils::Utf16ToUtf8String(password), password_hash)) {
				return H_AUTH_FAILED;
			}
			id_ = res->getInt("id");
			username_ = Utils::Utf8ToUtf16String(res->getString("username"));
		}
		statusStmt_->setInt(1, kStatus_Online);
		statusStmt_->setString(2, utf8email);
		statusStmt_->executeQuery();
		return H_OK;
	} catch (sql::SQLException& e) {
		return H_SERVER_DB_ERROR;
	}
}

int User::requestAddFriend(int userId)
{
	return H_OK;
}

int User::requestAddGroup(int groupId)
{
	return H_OK;
}

int User::removeFriend(int userId)
{
	return H_OK;
}

int User::quitGroup(int groupId)
{
	return H_OK;
}

int User::logOff()
{
	return H_OK;
}

int User::getStatus()
{
	return H_OK;
}

int User::createPreparedStatement()
{
	if (loginStmt_) {
		return H_OK;
	}
	auto con = ServerContext::getInstance()->getDBContext()->getDbConn();
	if (!con) {
		return H_SERVER_DB_ERROR;
	}
	try {
		loginStmt_.reset(con->prepareStatement("SELECT id,email,username,password_hash FROM User WHERE email=?"));
		if (!loginStmt_) {
			return H_SERVER_DB_ERROR;
		}
		statusStmt_.reset(con->prepareStatement("UPDATE user SET status=? WHERE email=?"));
		if (!statusStmt_) {
			return H_SERVER_DB_ERROR;
		}
		return H_OK;
	} catch (sql::SQLException& e) {
		return H_SERVER_DB_ERROR;
	}
}