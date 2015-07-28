#pragma once

class FileManager
{
public:
	FileManager();
	~FileManager();
	
private:
	std::map<int, std::wstring> fileRequestMap_;
};