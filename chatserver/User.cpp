#include "stdafx.h"
#include "../common/errcode.h"
#include "../common/StringUtils.h"
#include "../common/NetConstants.h"
#include "User.h"
#include "Utils.h"
#include "ServerContext.h"
#include "DBContext.h"

User::User()
{
}

User::~User()
{
}

HERRCODE User::login(int authType, const std::wstring& email, const std::wstring& credential)
{
	auto loginStmt = ServerContext::getInstance()->getDBContext()->getLoginStmt();
	auto statusStmt = ServerContext::getInstance()->getDBContext()->getStatusStmt();
	try {
		auto utf8email = StringUtils::Utf16ToUtf8String(email);
		loginStmt->setString(1, utf8email);
		std::unique_ptr<sql::ResultSet> res(loginStmt->executeQuery());
		if (res->rowsCount() != 1) {
			return H_AUTH_FAILED;
		}
		while (res->next()) {
			if (authType == net::kLoginType_Normal) {
				auto password_hash = res->getString("password_hash");
				if (!Utils::ValidatePasswordHash(StringUtils::Utf16ToUtf8String(credential), password_hash)) {
					return H_AUTH_FAILED;
				}
			} else if (authType == net::kLoginType_Auto) {
				auto token = res->getString("access_token");
				if (token != StringUtils::Utf16ToUtf8String(credential)) {
					return H_AUTH_FAILED;
				}
			}
			id_ = res->getInt("id");
			username_ = StringUtils::Utf8ToUtf16String(res->getString("username"));
			authKey_ = StringUtils::Utf8ToUtf16String(res->getString("auth_key"));
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

std::wstring User::getAuthKey()
{
	return authKey_;
}

