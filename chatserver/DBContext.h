#pragma once

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <mysql_error.h>

class DBContext
{
public:
	DBContext();
	~DBContext();
	int init();
	sql::Connection* getDbConn();

private:
	std::unique_ptr<sql::Connection> dbConn_;
};