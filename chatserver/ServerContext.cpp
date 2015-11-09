#include "stdafx.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <string>
#include "../common/FileUtils.h"
#include "../common/StringUtils.h"
#include "../common/Trace.h"
#include "ServerContext.h"

static const char kImageDir[] = "/.avchat/image/";

ServerContext::ServerContext()
{
}

ServerContext::~ServerContext()
{
}

int ServerContext::init()
{
	auto pw = getpwuid(getuid());
	if (!pw) {
		LOG_ERROR("cannot get home directory\n");
		return H_FAILED;
	}
	auto homedir = pw->pw_dir;
	imageDir_ = homedir;
	imageDir_ += kImageDir;
	if (!FileUtils::MkDirs(imageDir_.c_str())) {
		LOG_ERROR("cannot create server imagedir %s\n", imageDir_.c_str());
		return H_FAILED;
	}
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
