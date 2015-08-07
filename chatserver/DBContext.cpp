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

sql::Connection* DBContext::getDbConn()
{
	return dbConn_.get();
}

// FIXME: ini read write
int DBContext::init()
{
	std::string url = "tcp://127.0.0.1:3307";
	std::string user = "root";
	std::string pass = "2264seen";
	std::string dbName = "chat";
	try {
		sql::Driver * driver = sql::mysql::get_driver_instance();
		auto conn = driver->connect(url, user, pass);
		if (conn) {
			dbConn_.reset(conn);
			std::unique_ptr<sql::Statement> stmt(conn->createStatement());
			stmt->execute("USE " + dbName);
			return H_OK;
		}
		return H_SERVER_DB_ERROR;
	} catch (sql::SQLException& e) {
		return H_SERVER_DB_ERROR;
	}
}