#pragma once

#include "SockStream.h"
#include "errcode.h"
#include <string>
namespace base
{
	class Utils
	{
	public:
		Utils();
		~Utils();
		static unsigned int GetCpuCount();
		static std::u16string GenerateRandomString(int len);
		static std::u16string XorString(const std::u16string& str1, const std::u16string& str2);
		static int MakeSocketNonBlocking(int fd);
	};
}
