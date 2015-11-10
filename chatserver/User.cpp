#include "stdafx.h"
#include "../common/errcode.h"
#include "../common/NetConstants.h"
#include "User.h"

#include "../common/StringUtils.h"
#include "../common/FileUtils.h"
#include "../common/HashUtils.h"
#include "ServerContext.h"
#include "DBContext.h"

User::User()
{
	id_ = -1;
	status_ = -1;
}

User::~User()
{
}

HERRCODE User::login(int authType, const std::u16string& email, const std::u16string& credential)
{
	auto loginStmt = ServerContext::getInstance()->getDBContext()->getLoginStmt();
	auto statusStmt = ServerContext::getInstance()->getDBContext()->getStatusStmt();
	try {
		auto utf8email = su::u16to8(email);
		loginStmt->setString(1, utf8email);
		std::unique_ptr<sql::ResultSet> res(loginStmt->executeQuery());
		if (res->rowsCount() != 1) {
			return H_AUTH_FAILED;
		}
		while (res->next()) {
			if (authType == net::kLoginType_Normal) {
				auto password_hash = res->getString("password_hash");
				if (!base::HashUtils::ValidatePasswordHash(su::u16to8(credential), password_hash)) {
					return H_AUTH_FAILED;
				}
			} else if (authType == net::kLoginType_Auto) {
				auto authKey = res->getString("auth_key");
				if (authKey != su::u16to8(credential)) {
					return H_AUTH_FAILED;
				}
			}
			id_ = res->getInt("id");
			username_ = su::u8to16(res->getString("username"));
			authKey_ = su::u8to16(res->getString("auth_key"));
		}
		statusStmt->setInt(1, kStatus_Online);
		statusStmt->setString(2, utf8email);
		statusStmt->executeQuery();
		email_ = email;
		return H_OK;
	} catch (sql::SQLException& e) {
		return H_SERVER_DB_ERROR;
	}
}

HERRCODE User::logout()
{
	return H_OK;
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

int User::getStatus()
{
	return H_OK;
}

std::u16string User::getAuthKey() const
{
	return authKey_;
}

