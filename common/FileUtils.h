#pragma once

#include <stdint.h>
#include <string>
#include "buffer.h"

class FileUtils
{
public:
	FileUtils();
	~FileUtils();
	static bool FileExists(const char* szPath);
	static bool DirExists(const char* szPath);
	static bool PathExists(const char* szPath);
	static int64_t FnGetFileSize(const std::string& filePath);
	static bool ReadAll(const std::string& filePath, buffer& outBuf);
	static bool FnCreateFile(const std::string& filePath, buffer& buf);
	static int CalculateFileSHA1(const std::string& filePath, std::string* pHash);
	static std::string getFileExt(const std::string& filePath);
	static std::string FileSizeToReadable(int fileSize);
	static bool MkDirs(const std::string& filePath);
};
