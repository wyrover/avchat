#pragma once
#include <map>
#include <string>

class FileManager
{
public:
	FileManager();
	~FileManager();
	
private:
	std::map<int, std::u16string> fileRequestMap_;
};
