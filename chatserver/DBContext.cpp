#include "stdafx.h"
#include "DBContext.h"
#include <cppconn/statement.h>
#include <iostream>
#include <libconfig.h>
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
	config_t cfg;
	config_setting_t *settings;
	const char *str;
	config_init(&cfg);
	if (!config_read_file(&cfg, "avchat.cfg")) {
		perror("cannot find configure file\n");
		config_destroy(&cfg);
		return H_SERVER_ERROR;
	}
	std::string url;
	std::string user;
	std::string pass;
	std::string dbName;
	if (config_lookup_string(&cfg, "url", &str)) {
		url = str;
	} else {
		perror("db url missing\n");
		config_destroy(&cfg);
		return H_SERVER_ERROR;
	}

	if (config_lookup_string(&cfg, "user", &str)) {
		user = str;
	} else {
		perror("db user missing\n");
		config_destroy(&cfg);
		return H_SERVER_ERROR;
	}

	if (config_lookup_string(&cfg, "password", &str)) {
		pass = str;
	} else {
		perror("db password missing\n");
		config_destroy(&cfg);
		return H_SERVER_ERROR;
	}

	if (config_lookup_string(&cfg, "dbName", &str)) {
		dbName = str;
	} else {
		perror("db password missing\n");
		config_destroy(&cfg);
		return H_SERVER_ERROR;
	}

	config_destroy(&cfg);

	try {
		sql::Driver * driver = sql::mysql::get_driver_instance();
		auto conn = driver->connect(url, user, pass);
		if (!conn) {
			perror("cannot connect to mysql\n");
			return H_SERVER_DB_ERROR;
		}
		dbConn_.reset(conn);
		std::unique_ptr<sql::Statement> stmt(conn->createStatement());
		stmt->execute("USE " + dbName);
		auto hr = createPreparedStatement();
		return hr;
	} catch (sql::SQLException& e) {
		perror("mysql error\n");
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
		loginStmt_.reset(dbConn_->prepareStatement("SELECT id,email,username,password_hash,auth_key FROM user WHERE email=?"));
		if (!loginStmt_) {
			perror("cannot create prepared statement User\n");
			return H_SERVER_DB_ERROR;
		}
		statusStmt_.reset(dbConn_->prepareStatement("UPDATE user SET status=? WHERE email=?"));
		if (!statusStmt_) {
			perror("cannot create prepared statement User\n");
			return H_SERVER_DB_ERROR;
		}
		getFileUrlStmt_.reset(dbConn_->prepareStatement("SELECT url FROM filemap WHERE hash=?"));
		if (!getFileUrlStmt_) {
			perror("cannot create prepared statement filemap\n");
			return H_SERVER_DB_ERROR;
		}
		addFileStmt_.reset(dbConn_->prepareStatement("INSERT INTO filemap(hash,url) VALUES(?, ?)"));
		if (!addFileStmt_) {
			perror("cannot create prepared statement filemap\n");
					return H_SERVER_DB_ERROR;
		}
		return H_OK;
	} catch (sql::SQLException& e) {
		std::cerr << "#\t\t " << e.what() << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
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

HERRCODE DBContext::getFileUrl(const std::string& hashId, std::string* url)
{
	url->clear();
	try {
		getFileUrlStmt_->setString(1, hashId);
		std::unique_ptr<sql::ResultSet> res(getFileUrlStmt_->executeQuery());
		if (res->rowsCount() != 1) {
			return H_FAILED;
		}
		while (res->next()) {
			auto name = res->getString("url");
			if (!name->empty()) {
				*url = ServerContext::getInstance()->getImageDir() + name;
				return H_OK;
			}
		}
		return H_FAILED;
	} catch (sql::SQLException& e) {
		return H_SERVER_DB_ERROR;
	}
}

HERRCODE DBContext::addFile(const std::string& hashId, const std::string& url)
{
	try {
		addFileStmt_->setString(1, hashId);
		addFileStmt_->setString(2, url);
		addFileStmt_->executeUpdate();
		return H_OK;
	} catch (sql::SQLException& e) {
		return H_SERVER_DB_ERROR;
	}
}
