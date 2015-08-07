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

private:
	DBContext db_;
};