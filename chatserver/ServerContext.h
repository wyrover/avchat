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
	std::wstring getImageDir();

private:
	DBContext db_;
	std::wstring imageDir_;
};