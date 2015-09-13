#include "stdafx.h"
#include "Utils.h"
#include "ChatOverlappedData.h"
#include "NetConstants.h"
#include <assert.h>
#include <random>

namespace base
{
	Utils::Utils()
	{
	}

	Utils::~Utils()
	{
	}

	unsigned int Utils::GetCpuCount()
	{
		unsigned count = std::thread::hardware_concurrency();
		if (count != 0)
			return count;
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		return sysinfo.dwNumberOfProcessors;
	}

	HERRCODE Utils::QueueSendRequest(SOCKET socket, SockStream& stream, HANDLE hComp, DWORD compKey)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = stream.getBuf();
		wsaBuf.len = stream.getSize();
		ChatOverlappedData* ol = new ChatOverlappedData(net::kAction_Send);
		int rc = WSASend(socket, &wsaBuf, 1, NULL, 0, ol, NULL);
		if (rc != 0) {
			int errcode = WSAGetLastError();
			if (errcode != WSA_IO_PENDING) {
				return H_FAILED;
			} else {
				PostQueuedCompletionStatus(hComp, rc, compKey, ol);
			}
		}
		return H_OK;
	}

	HERRCODE Utils::QueueSendRequest(SOCKET socket, const sockaddr_in& remoteAddr, SockStream& stream, HANDLE hComp, DWORD compKey)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = stream.getBuf();
		wsaBuf.len = stream.getSize();
		ChatOverlappedData* ol = new ChatOverlappedData(net::kAction_Send);
		int rc = WSASendTo(socket, &wsaBuf, 1, NULL, 0, (const sockaddr*)&remoteAddr, sizeof(sockaddr_in), ol, NULL);
		if (rc != 0) {
			int errcode = WSAGetLastError();
			if (errcode != WSA_IO_PENDING) {
				return H_FAILED;
			} else {
				PostQueuedCompletionStatus(hComp, rc, compKey, ol);
			}
		}
		return H_OK;
	}

	HERRCODE Utils::QueueRecvCmdRequest(SOCKET socket, HANDLE hComp, DWORD compKey)
	{
		int type;
		int length = sizeof(int);
		getsockopt(socket, SOL_SOCKET, SO_TYPE, (char*)&type, &length);

		auto ol = new ChatOverlappedData(net::kAction_Recv);
		ol->setSocket(socket);
		buffer& buf = ol->getBuf();
		WSABUF wsaBuf;
		wsaBuf.buf = buf.data();
		wsaBuf.len = buf.size();
		DWORD flags = 0;
		int rc = 0;
		if (type == SOCK_STREAM) {
			rc = WSARecv(socket, &wsaBuf, 1, NULL, &flags, ol, NULL);
		} else {
			auto addr = ol->getAddr();
			auto len = ol->getAddrLen();
			rc = WSARecvFrom(socket, &wsaBuf, 1, NULL, &flags, (struct sockaddr*)addr, len, ol, NULL);
		}
		if (rc != 0) {
			int errcode = WSAGetLastError();
			if (rc != WSA_IO_PENDING)
				return H_FAILED;
			else {
				PostQueuedCompletionStatus(hComp, rc, compKey, ol);
			}
		}
		return H_OK;
	}

	std::wstring Utils::GenerateRandomString(int len)
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<double> dist(0, 26);
		std::wstring result;
		for (int i = 0; i < len; ++i)  {
			result.push_back(L'A' + dist(mt));
		}
		return result;
	}

	std::wstring Utils::XorString(const std::wstring& str1, const std::wstring& str2)
	{
		assert(str1.size() == str2.size());
		std::wstring result;
		result.resize(str1.size());
		for (int i = 0; i < str1.size(); ++i) {
			unsigned short ch1 = str1[i];
			unsigned short ch2 = str2[i];
			result[i] = ch1 ^ ch2;
		}
		return result;
	}
}