#include "stdafx.h"
#include "DBContext.h"
#include <cppconn/statement.h>
#include "../common/errcode.h"
#include "../common/StringUtils.h"
#include "Utils.h"
#include "ServerContext.h"

DBContext::DBContext()
{
}

DBContext::~DBContext()
{
}

// FIXME: ini read write
HERRCODE DBContext::init()
{
	std::string url = "tcp://127.0.0.1:3306";
	std::string user = "root";
	std::string pass = "";
	std::string dbName = "avchat";
	try {
		sql::Driver * driver = sql::mysql::get_driver_instance();
		auto conn = driver->connect(url, user, pass);
		if (!conn)
			return H_SERVER_DB_ERROR;
		dbConn_.reset(conn);
		std::unique_ptr<sql::Statement> stmt(conn->createStatement());
		stmt->execute("USE " + dbName);
		auto hr = createPreparedStatement();
		return hr;
	} catch (sql::SQLException& e) {
		return H_SERVER_DB_ERROR;
	}
}

HERRCODE DBContext::createPreparedStatement()
{
	if (loginStmt_) {
		return H_OK;
	}
	if (!dbConn_) {
		return H_SERVER_DB_ERROR;
	}
	try {
		loginStmt_.reset(dbConn_->prepareStatement("SELECT id,email,username,password_hash,auth_key FROM User WHERE email=?"));
		if (!loginStmt_) {
			return H_SERVER_DB_ERROR;
		}
		statusStmt_.reset(dbConn_->prepareStatement("UPDATE user SET status=? WHERE email=?"));
		if (!statusStmt_) {
			return H_SERVER_DB_ERROR;
		}
		getFileUrlStmt_.reset(dbConn_->prepareStatement("SELECT url FROM filemap WHERE hash=?"));
		if (!getFileUrlStmt_) {
			return H_SERVER_DB_ERROR;
		}
		addFileStmt_.reset(dbConn_->prepareStatement("INSERT INTO filemap(hash,url) VALUES(?, ?)"));
		if (!addFileStmt_) {
			return H_SERVER_DB_ERROR;
		}
		return H_OK;
	} catch (sql::SQLException& e) {
		return H_SERVER_DB_ERROR;
	}
}

sql::PreparedStatement* DBContext::getLoginStmt()
{
	return loginStmt_.get();
}

sql::PreparedStatement* DBContext::getStatusStmt()
{
	return statusStmt_.get();
}

HERRCODE DBContext::getFileUrl(const std::wstring& hashId, std::wstring* url)
{
	url->clear();
	try {
		getFileUrlStmt_->setString(1, StringUtils::Utf16ToUtf8String(hashId));
		std::unique_ptr<sql::ResultSet> res(getFileUrlStmt_->executeQuery());
		if (res->rowsCount() != 1) {
			return H_FAILED;
		}
		while (res->next()) {
			auto name = res->getString("url");
			if (!name->empty()) {
				*url = ServerContext::getInstance()->getImageDir() + StringUtils::Utf8ToUtf16String(name);
				return H_OK;
			}
		}
		return H_FAILED;
	} catch (sql::SQLException& e) {
		return H_SERVER_DB_ERROR;
	}
}

HERRCODE DBContext::addFile(const std::wstring& hashId, const std::wstring& url)
{
	try {
		addFileStmt_->setString(1, StringUtils::Utf16ToUtf8String(hashId));
		addFileStmt_->setString(2, StringUtils::Utf16ToUtf8String(url));
		addFileStmt_->executeUpdate();
		return H_OK;
	} catch (sql::SQLException& e) {
		return H_SERVER_DB_ERROR;
	}
}