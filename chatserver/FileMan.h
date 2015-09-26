#pragma once

class FileMan
{
public:
	FileMan();
	~FileMan();
	HERRCODE addFile(buffer& buf, const std::u16string& fileExt, std::u16string* url);
	HERRCODE getFileUrl(const std::u16string& hashId, std::u16string* url);
	HERRCODE getFile(const std::u16string& url, buffer& outBuf);
};
