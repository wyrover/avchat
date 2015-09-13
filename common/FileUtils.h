#pragma once
#include <stdint.h>
#include <string>
#include <windows.h>
#include "buffer.h"
class FileUtils
{
public:
	FileUtils();
	~FileUtils();
	static bool FileExists(LPCWSTR szPath);
	static bool DirExists(LPCWSTR szPath);
	static bool PathExists(LPCWSTR szPath);
	static int64_t GetFolderSize(const std::wstring& dirPath);
	static int64_t FnGetFileSize(const std::wstring& filePath);
	static bool ReadAll(const std::wstring& filePath, buffer& outBuf);
	static bool FnCreateFile(const std::wstring& filePath, buffer& buf);
	static int CalculateFileSHA1(const std::wstring& filePath, std::wstring* pHash);
	static std::wstring getFileExt(const std::wstring& filePath);
	static void MkDirs(const std::wstring& dirPath);
	static std::wstring FileSizeToReadable(int fileSize);
};