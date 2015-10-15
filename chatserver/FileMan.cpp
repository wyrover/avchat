#include "stdafx.h"
#include "../common/errcode.h"
#include "../common/FileUtils.h"
#include "FileMan.h"

#include "../common/StringUtils.h"
#include "Utils.h"
#include "ServerContext.h"
#include "DBContext.h"

FileMan::FileMan()
{
}

FileMan::~FileMan()
{
}

HERRCODE FileMan::addFile(buffer& buf, const std::u16string& fileExt, std::u16string* url)
{
	if (!Utils::IsImage(buf))
		return H_INVALID_FORMAT;
	if (!Utils::IsImageExt(fileExt))
		return H_INVALID_FORMAT;
	std::string filePath = ServerContext::getInstance()->getImageDir();
	auto fileName = su::u16to8(Utils::GetRandomFileName() + u"." + fileExt);
	filePath += fileName;
	if (!FileUtils::FnCreateFile(filePath, buf)) {
		return H_FAILED;
	}
	std::string hashId;
	if (FileUtils::CalculateFileSHA1(filePath, &hashId) != 0)
		return H_FAILED;
	auto hr = ServerContext::getInstance()->getDBContext()->addFile(hashId, fileName);
	if (hr == H_OK) {
		*url = su::u8to16(fileName);
	}
	return hr;
}

HERRCODE FileMan::getFileUrl(const std::u16string& hashId, std::u16string* url)
{
	std::string utf8Url;
	auto rc = ServerContext::getInstance()->getDBContext()->getFileUrl(su::u16to8(hashId), &utf8Url);
	*url = su::u8to16(utf8Url);
	return rc;
}

HERRCODE FileMan::getFile(const std::u16string& url, buffer& outBuf)
{
	std::string filePath = ServerContext::getInstance()->getImageDir();
	filePath += su::u16to8(url);
	if (!FileUtils::ReadAll(filePath, outBuf))
		return H_FAILED;
	return H_OK;
}
