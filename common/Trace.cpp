#include "stdafx.h"
#include <wtypes.h>
#include <tchar.h>
#include <wchar.h>
#include <time.h>
#include <vector>
#include <string.h>

static const int kBufferLen = 1024;

void Trace(const char* fileName, const int line, const char* funName, const wchar_t* format, ...)
{
	wchar_t buff[kBufferLen] = { 0 };
	bool hasCr = false;
	SYSTEMTIME t;
	GetLocalTime(&t);
	swprintf_s(buff, L"[%02d:%02d:%02d:%d]\t", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);

	va_list argList;
	va_start(argList, format);
	_vsnwprintf_s((wchar_t*)(buff + wcslen(buff)), kBufferLen - wcslen(buff), kBufferLen - wcslen(buff), format, argList);
	va_end(argList);

	if (buff[wcslen(buff) - 1] == L'\n') {
		buff[wcslen(buff) - 1] = '\0';
		hasCr = true;
	}

	size_t len = 0;
	mbstowcs_s(&len, 0, 0, fileName, 0);
	std::vector<wchar_t> wfileName(len + 1);
	mbstowcs_s(&len, wfileName.data(), len, fileName, len);
	wfileName[len] = 0;

	mbstowcs_s(&len, 0, 0, funName, 0);
	std::vector<wchar_t> wfunName(len + 1);
	mbstowcs_s(&len, wfunName.data(), len, funName, len);
	wfunName[len] = 0;

	swprintf_s(buff + wcslen(buff), kBufferLen - wcslen(buff), L"\t(%s:%d:%s)", wfileName.data(), line, wfunName.data());
	if (hasCr)
		wcscat_s(buff, L"\n");
	OutputDebugString(buff);
}

void Trace(const char* fileName, const int line, const char* funName, const char* format, ...)
{
	char buff[kBufferLen] = { 0 };
	bool hasCr = false;
	SYSTEMTIME t;
	GetLocalTime(&t);
	sprintf_s(buff, "[%02d:%02d:%02d:%d]\t", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);

	va_list argList;
	va_start(argList, format);
	_vsnprintf_s((char*)(buff + strlen(buff)), kBufferLen - strlen(buff), kBufferLen - strlen(buff), format, argList);
	va_end(argList);

	if (buff[strlen(buff) - 1] == L'\n') {
		buff[strlen(buff) - 1] = '\0';
		hasCr = true;
	}
	sprintf_s(buff + strlen(buff), kBufferLen - strlen(buff), "\t(%s:%d:%s)", fileName, line, funName);
	if (hasCr) {
		strcat_s(buff, "\n");
	}
	OutputDebugStringA(buff);
}