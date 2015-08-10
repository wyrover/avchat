#include "stdafx.h"
#include "DBContext.h"
#include <cppconn/statement.h>
#include "../common/errcode.h"

DBContext::DBContext()
{
}

DBContext::~DBContext()
{
}

// FIXME: ini read write
HERRCODE DBContext::init()
{
	std::string url = "tcp://127.0.0.1:3307";
	std::string user = "root";
	std::string pass = "2264seen";
	std::string dbName = "chat";
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
		loginStmt_.reset(dbConn_->prepareStatement("SELECT id,email,username,password_hash FROM User WHERE email=?"));
		if (!loginStmt_) {
			return H_SERVER_DB_ERROR;
		}
		statusStmt_.reset(dbConn_->prepareStatement("UPDATE user SET status=? WHERE email=?"));
		if (!statusStmt_) {
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