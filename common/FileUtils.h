#pragma once
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
};