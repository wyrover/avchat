#include "stdafx.h"
#include <assert.h>
#include <random>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
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

	HERRCODE Utils::BindSocket(int sock, const std::string& ip, const std::string& port)
	{
		addrinfo hints;
		addrinfo *result;
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;
		if (getaddrinfo(ip.c_str(), port.c_str(), &hints, &result) != 0) {
			return H_NETWORK_ERROR;
		}

		int rc = -1;
		for (auto rp = result; rp != nullptr; rp = rp->ai_next) {
			rc = bind(sock, rp->ai_addr, rp->ai_addrlen);
			if (rc != 0) {
				perror("bind failed\n");
				break;
			}
		}

		if (rc != 0) {
			return H_NETWORK_ERROR;
		}
		return H_OK;
	}

}
