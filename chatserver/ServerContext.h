#pragma once

#include "DBContext.h"

class ServerContext
{
public:
	ServerContext();
	~ServerContext();
	int init();
	DBContext* getDBContext();
	static ServerContext* getInstance();
	std::string getImageDir();

private:
	DBContext db_;
	std::string imageDir_;
};
