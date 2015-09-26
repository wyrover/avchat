#include "stdafx.h"
#include <assert.h>
#include <random>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <boost/locale/encoding_utf.hpp>
#include "Utils.h"
#include "NetConstants.h"

namespace base
{
	Utils::Utils()
	{
	}

	Utils::~Utils()
	{
	}

	int Utils::MakeSocketNonBlocking(int sfd)
	{
		int flags, s;

		flags = fcntl (sfd, F_GETFL, 0);
		if (flags == -1)
		{
			perror ("fcntl");
			return -1;
		}

		flags |= O_NONBLOCK;
		s = fcntl (sfd, F_SETFL, flags);
		if (s == -1)
		{
			perror ("fcntl");
			return -1;
		}
		return 0;
	}

	unsigned int Utils::GetCpuCount()
	{
		unsigned count = std::thread::hardware_concurrency();
		return count;
	}

	std::u16string Utils::GenerateRandomString(int len)
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<double> dist(0, 26);
		std::u16string result;
		for (int i = 0; i < len; ++i)  {
			result.push_back(u'A' + dist(mt));
		}
		return result;
	}

	std::u16string Utils::XorString(const std::u16string& str1, const std::u16string& str2)
	{
		assert(str1.size() == str2.size());
		std::u16string result;
		result.resize(str1.size());
		for (int i = 0; i < str1.size(); ++i) {
			unsigned short ch1 = str1[i];
			unsigned short ch2 = str2[i];
			result[i] = ch1 ^ ch2;
		}
		return result;
	}

	static std::string Utf16ToUtf8String(const std::u16string& str)
	{
		boost::locale::conv::utf_to_utf<char>(str.c_str(), str.c_str() + str.length());
	}

	static std::u16string Utf8ToUtf16String(const std::string& str)
	{
		boost::locale::conv::utf_to_utf<char16_t>(str.c_str(), str.c_str() + str.length());
	}
}
