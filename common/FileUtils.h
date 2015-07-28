#pragma once
class FileUtils
{
public:
	FileUtils();
	~FileUtils();
	static bool FileExists(LPCWSTR szPath);
	static bool DirExists(LPCWSTR szPath);
	static bool PathExists(LPCWSTR szPath);

};

