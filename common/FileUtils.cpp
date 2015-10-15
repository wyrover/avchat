#include "stdafx.h"
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>
#include "FileUtils.h"

FileUtils::FileUtils()
{
}


FileUtils::~FileUtils()
{
}

bool FileUtils::DirExists(const char* dirPath)
{
	struct stat sb;
	return (stat(dirPath, &sb) == 0) && S_ISDIR(sb.st_mode);
}

bool FileUtils::FileExists(const char* filePath)
{
	struct stat sb;
	return (stat(filePath, &sb) == 0) && S_ISREG(sb.st_mode);
}

bool FileUtils::PathExists(const char* path)
{
	struct stat sb;
	return (stat(path, &sb) == 0) && (S_ISDIR(sb.st_mode) || S_ISREG(sb.st_mode));
}

int64_t FileUtils::FnGetFileSize(const std::string& filePath)
{
	FILE* fd = fopen(filePath.c_str(),"rb"); 
	if (fd == NULL)
		return false;
	fseek(fd, 0, SEEK_END);
	auto fileSize = ftell(fd);
	fclose(fd);
	return fileSize;
}

bool FileUtils::ReadAll(const std::string& filePath, buffer& outBuf)
{
	buffer readBuf;
	FILE* fd = fopen(filePath.c_str(),"rb"); 
	if (fd == NULL)
		return false;
	fseek(fd, 0, SEEK_END);
	auto fileSize = ftell(fd);
	if (fileSize <= 0) {
		fclose(fd);
		return false;
	}
	fseek(fd, 0, SEEK_SET);
	readBuf.size(fileSize);
	auto rc = fread(readBuf.data(), fileSize, 1, fd);
	fclose(fd);
	if (rc != fileSize) {
		return false;
	} else {
		readBuf.swap(outBuf);
		return true;
	}
}

bool FileUtils::FnCreateFile(const std::string& filePath, buffer& buf)
{
	buffer readBuf;
	auto fd = fopen(filePath.c_str(),"wb"); 
	if (fd == NULL)
		return false;
	auto rc = fwrite(buf.data(), buf.size(), 1, fd);
	fclose(fd);
	return rc == buf.size();
}

int FileUtils::CalculateFileSHA1(const std::string& filePath, std::string* pHash)
{
	std::string result;
	CryptoPP::SHA1 hash;
	CryptoPP::FileSource(filePath.c_str(), true, 
			new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(
					new CryptoPP::StringSink(result), true)));
	*pHash = result;
	return 0;
}

std::string FileUtils::getFileExt(const std::string& filePath)
{
	auto pos = filePath.rfind(".");
	if (pos == -1)
		return "";
	return filePath.substr(pos);
}

std::string FileUtils::FileSizeToReadable(int fileSize)
{
	char buf[20];
	double size = fileSize;
	int i = 0;
	const char* units[] = { "B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
	while (size > 1024) {
		size /= 1024;
		i++;
	}
	sprintf(buf, "%.*f %s", i, size, units[i]);
	return buf;
}

void FileUtils::MkDirs(const std::string& filePath)
{
	char tmp[PATH_MAX];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp),"%s",filePath.c_str());
	len = strlen(tmp);
	if(tmp[len - 1] == '/')
		tmp[len - 1] = 0;
	for(p = tmp + 1; *p; p++)
		if(*p == '/') {
			*p = 0;
			mkdir(tmp, S_IRWXU);
			*p = '/';
		}
	mkdir(tmp, S_IRWXU);
}
