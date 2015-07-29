#include "stdafx.h"
#include "FileUtils.h"


FileUtils::FileUtils()
{
}


FileUtils::~FileUtils()
{
}

bool FileUtils::DirExists(LPCWSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool FileUtils::FileExists(LPCWSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool FileUtils::PathExists(LPCWSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES);
}

int64_t FileUtils::GetFolderSize(const std::wstring& dirPath)
{
	int64_t size = 0;
	if (!DirExists(dirPath.c_str())) {
		return -1;
	} else {
		WIN32_FIND_DATA findData;
		HANDLE hFind = FindFirstFile(dirPath.c_str(), &findData);
		if (hFind == INVALID_HANDLE_VALUE)
			return -1;
		do {
			if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				size += GetFolderSize(findData.cFileName);
			} else {
				HANDLE hFile = CreateFile(findData.cFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
				if (hFile != INVALID_HANDLE_VALUE) {
					LARGE_INTEGER li = { 0 };
					GetFileSizeEx(hFile, &li);
					size += li.QuadPart;
					CloseHandle(hFile);
				}
			}
		} while (FindNextFile(hFind, &findData) != 0);
		FindClose(hFind);
	}
	return size;
}

int64_t FileUtils::FnGetFileSize(const std::wstring& filePath)
{
	HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	LARGE_INTEGER li;
	bool result = !!GetFileSizeEx(hFile, &li);
	CloseHandle(hFile);
	if (!result) {
		return -1;
	} else {
		return li.QuadPart;
	}
}