#include "stdafx.h"
#include "ServerContext.h"

ServerContext::ServerContext()
{
	WCHAR szPath[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath);
	PathAppend(szPath, L"\\fakecoder\\server_cache\\");
	SHCreateDirectoryEx(NULL, szPath, NULL);
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

std::wstring ServerContext::getImageDir()
{
	return imageDir_;
}
