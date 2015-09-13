#pragma once

#include "SockStream.h"
#include "errcode.h"

namespace base
{
	class Utils
	{
	public:
		Utils();
		~Utils();
		static unsigned int GetCpuCount();
		static HERRCODE QueueSendRequest(SOCKET socket, SockStream& stream, HANDLE hComp, DWORD compKey);
		static HERRCODE QueueSendRequest(SOCKET socket, const sockaddr_in& remoteAddr, SockStream& stream, HANDLE hComp, DWORD compKey);
		static HERRCODE QueueRecvCmdRequest(SOCKET socket, HANDLE hComp, DWORD compKey);
		static std::wstring GenerateRandomString(int len);
		static std::wstring XorString(const std::wstring& str1, const std::wstring& str2);
	};
}
