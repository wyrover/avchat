#include "stdafx.h"
#include "ServerContext.h"

ServerContext::ServerContext()
{
}

ServerContext::~ServerContext()
{
}

// FIXME: ini read write
int ServerContext::init()
{
	return db_.init();
}

ServerContext* ServerContext::getInstance()
{
	static ServerContext context;
	return &context;
}

DBContext* ServerContext::getDBContext()
{
	return &db_;
}