#include "stdafx.h"
#include "../common/errcode.h"
#include "../common/FileUtils.h"
#include "FileMan.h"
#include "Utils.h"
#include "ServerContext.h"
#include "DBContext.h"
#include <strsafe.h>
#include <Shlwapi.h>

FileMan::FileMan()
{
}

FileMan::~FileMan()
{
}

HERRCODE FileMan::addFile(buffer& buf, const std::wstring& fileExt, std::wstring* url)
{
	if (!Utils::IsImage(buf))
		return H_INVALID_FORMAT;
	if (!Utils::IsImageExt(fileExt))
		return H_INVALID_FORMAT;
	WCHAR path[MAX_PATH];
	StringCchCopy(path, MAX_PATH, ServerContext::getInstance()->getImageDir().c_str());
	auto fileName = Utils::GetRandomFileName() + L"." + fileExt;
	PathAppend(path, fileName.c_str());
	if (!FileUtils::FnCreateFile(path, buf)) {
		return H_FAILED;
	}
	std::wstring hashId;
	if (FileUtils::CalculateFileSHA1(path, &hashId) != 0)
		return H_FAILED;
	auto hr = ServerContext::getInstance()->getDBContext()->addFile(hashId, fileName);
	if (hr == H_OK) {
		*url = fileName;
	}
	return hr;
}

HERRCODE FileMan::getFileUrl(const std::wstring& hashId, std::wstring* url)
{
	return ServerContext::getInstance()->getDBContext()->getFileUrl(hashId, url);
}

HERRCODE FileMan::getFile(const std::wstring& url, buffer& outBuf)
{
	WCHAR path[MAX_PATH];
	StringCchCopy(path, MAX_PATH, ServerContext::getInstance()->getImageDir().c_str());
	PathAppend(path, url.c_str());
	if (!FileUtils::ReadAll(path, outBuf))
		return H_FAILED;
	return H_OK;
}
