#pragma once

#ifdef _WIN32
#include "win_trace.h"
#else
#include "linux_trace.h"
#endif

void Trace(const char* fileName, const int line, const char* funName, const char16_t* format, ...);
void Trace(const char* fileName, const int line, const char* funName, const char* format, ...);
void LogError(const char* fileName, const int line, const char* funName, const char* format, ...);

#define TRACE(format, ...) \
		do { \
			Trace(__SHORT_FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
		} while(0);

#define TRACE_IF(cond, format, ...) \
		{ \
	do { \
		if (cond) {\
			Trace(__SHORT_FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
		} \
	} while(0);}



#define LOG_ERROR(format, ...) \
		do { \
			LogError(__SHORT_FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
		} while(0);

