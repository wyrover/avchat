#pragma once

class FileMan
{
public:
	FileMan();
	~FileMan();
	HERRCODE addFile(buffer& buf, const std::wstring& fileExt, std::wstring* url);
	HERRCODE getFileUrl(const std::wstring& hashId, std::wstring* url);
	HERRCODE getFile(const std::wstring& url, buffer& outBuf);
};