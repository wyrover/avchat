#pragma once

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <mysql_error.h>
#include "../common/errcode.h"
class DBContext
{
public:
	DBContext();
	~DBContext();
	HERRCODE init();
	sql::PreparedStatement* getLoginStmt();
	sql::PreparedStatement* getStatusStmt();
	HERRCODE getFileUrl(const std::string& hashId, std::string* url);
	HERRCODE addFile(const std::string& hashId, const std::string& url);

private:
	std::unique_ptr<sql::Connection> dbConn_;
	std::unique_ptr<sql::PreparedStatement> loginStmt_;
	std::unique_ptr<sql::PreparedStatement> getFileUrlStmt_;
	std::unique_ptr<sql::PreparedStatement> statusStmt_;
	std::unique_ptr<sql::PreparedStatement> addFileStmt_;

private:
	HERRCODE createPreparedStatement();
};
