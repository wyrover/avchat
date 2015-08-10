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

private:
	std::unique_ptr<sql::Connection> dbConn_;
	std::unique_ptr<sql::PreparedStatement> loginStmt_;
	std::unique_ptr<sql::PreparedStatement> statusStmt_;

private:
	HERRCODE createPreparedStatement();
};