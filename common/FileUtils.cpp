#include "stdafx.h"
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>
#include <cryptopp/osrng.h>
#include "StringUtils.h"
#include "FileUtils.h"

namespace {
	static std::vector<unsigned char> kPngMagic = {
		0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a
	};

	static std::vector<unsigned char> kGifMagic1 = {
		0x47, 0x49, 0x46, 0x38, 0x39, 0x61
	};
	static std::vector<unsigned char> kGifMagic2 = {
		0x47, 0x49, 0x46, 0x38, 0x37, 0x61
	};
	static std::vector<unsigned char> kJpegExifMagic = {
		0xFF, 0xD8, 0xFF, 0xE1,
	};
	static std::vector<unsigned char> kJpegJfifMagic = {
		0xFF, 0xD8, 0xFF, 0xE0,
	};
	static std::vector<unsigned char> kBmpMagic = {
		0x42, 0x4d,
	};

	static std::vector<std::u16string> kImageExts = {
		u"jpg", u"jpeg", u"gif", u"png", u"bmp",
	};

	static bool compareMagic(buffer& buf, const std::vector<unsigned char>& magic) {
		if (buf.size() > magic.size()) {
			bool isImage = true;
			for (int i = 0; i < magic.size(); ++i) {
				if (magic[i] != (unsigned char)buf[i]) {
					isImage = false;
					break;
				}
			}
			if (isImage)
				return true;
		}
		return false;
	}

}

namespace base
{
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
		if (rc != 1) {
			return false;
		} else {
			readBuf.swap(outBuf);
			return true;
		}
	}

	bool FileUtils::FnCreateFile(const std::string& filePath, buffer& buf)
	{
		auto fd = fopen(filePath.c_str(),"wb"); 
		if (fd == NULL)
			return false;
		auto rc = fwrite(buf.data(), buf.size(), 1, fd);
		fclose(fd);
		return rc;
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

	bool FileUtils::MkDirs(const std::string& filePath)
	{
		std::vector<char> keke(filePath.begin(), filePath.end());
		char* tmp = keke.data();
		char *p = NULL;
		size_t len;
		len = filePath.size();
		if(tmp[len - 1] == '/')
			tmp[len - 1] = 0;
		for(p = tmp + 1; *p; p++) {
			if(*p == '/') {
				*p = 0;
				if (mkdir(tmp, S_IRWXU) != 0 && errno != EEXIST)
					return false;
				*p = '/';
			}
		}
		if (mkdir(tmp, S_IRWXU) != 0 && errno != EEXIST)
			return false;
		return true;
	}

	bool FileUtils::IsImage(buffer& buf)
	{
		if (compareMagic(buf, kPngMagic))
			return true;
		if (compareMagic(buf, kGifMagic1))
			return true;
		if (compareMagic(buf, kGifMagic2))
			return true;
		if (compareMagic(buf, kJpegJfifMagic))
			return true;
		if (compareMagic(buf, kJpegExifMagic))
			return true;
		if (compareMagic(buf, kBmpMagic))
			return true;
		return false;
	}


	bool FileUtils::IsImageExt(const std::u16string& aExt)
	{
		if (aExt.empty())
			return false;
		auto ext = aExt;
		if (aExt[0] == '.')
			ext = aExt.substr(1);

		return std::find_if(kImageExts.begin(), kImageExts.end(), [&ext](const std::u16string& item) {
				return su::tolower(item) == su::tolower(ext); }) != kImageExts.end();
	}

	std::u16string FileUtils::GetRandomFileName()
	{
		using namespace CryptoPP;
		AutoSeededRandomPool rng;
		std::vector<byte> buffer(13);
		rng.GenerateBlock(buffer.data(), buffer.size());
		std::u16string fileName;
		for (int i = 0; i < buffer.size(); i++) {
			// restrict to length of range [a..z0..9]
			int b = (buffer[i] % 36);
			wchar_t c = (char)(b < 26 ? (b + L'a') : (b - 26 + L'0'));
			fileName += c;
		}
		return fileName;
	}

}
