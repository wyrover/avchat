#include "stdafx.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <string>
#include "ServerContext.h"

static const char kImageDir[] = "avchat/image/";
ServerContext::ServerContext()
{
	auto pw = getpwuid(getuid());
	auto homedir = pw->pw_dir;
	imageDir_ = homedir;
	imageDir_ += kImageDir;
	mkdir(imageDir_.c_str(), 0666);
}

ServerContext::~ServerContext()
{
}

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

std::string ServerContext::getImageDir()
{
	return imageDir_;
}
