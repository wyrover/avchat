#include "stdafx.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <vector>
#include <string>

static const int kBufferLen = 1024;

void Trace(const char* fileName, const int line, const char* funName, const char16_t* format, ...)
{

}

void Trace(const char* fileName, const int line, const char* funName, const char* format, ...)
{
	char buff[kBufferLen] = { 0 };
	bool hasCr = false;
	time_t timeTick = time(nullptr);
	tm timeTm;
	localtime_r(&timeTick, &timeTm);
	snprintf(buff, kBufferLen, "[%02d:%02d:%02d]\t", timeTm.tm_hour, timeTm.tm_min, timeTm.tm_sec);

	va_list argList;
	va_start(argList, format);
	vsnprintf((char*)(buff + strlen(buff)), kBufferLen - strlen(buff), format, argList);
	va_end(argList);

	if (buff[strlen(buff) - 1] == L'\n') {
		buff[strlen(buff) - 1] = '\0';
		hasCr = true;
	}
	snprintf(buff + strlen(buff), kBufferLen - strlen(buff), "\t(%s:%d:%s)", fileName, line, funName);
	if (hasCr) {
		strcat(buff, "\n");
	}
	fprintf(stdout, buff);
}
