#include "stdafx.h"
#include "Utils.h"
#include "ChatOverlappedData.h"
#include "NetConstants.h"

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

	HERRCODE Utils::QueueSendRequest(SOCKET socket, SockStream& stream)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = stream.getBuf();
		wsaBuf.len = stream.getSize();
		ChatOverlappedData* ol = new ChatOverlappedData(net::kAction_Send);
		int ret = WSASend(socket, &wsaBuf, 1, NULL, 0, ol, NULL);
		int errcode = WSAGetLastError();
		if (ret != 0) {
			assert(errcode == WSA_IO_PENDING);
			return H_FAILED;
		}
		return H_OK;
	}

	HERRCODE Utils::QueueRecvCmdRequest(SOCKET socket)
	{
		auto ol = new ChatOverlappedData(net::kAction_Recv);
		ol->setSocket(socket);
		buffer& buf = ol->getBuf();
		WSABUF wsaBuf;
		wsaBuf.buf = buf.data();
		wsaBuf.len = buf.size();
		DWORD flags = 0;
		int err = WSARecv(socket, &wsaBuf, 1, NULL, &flags, ol, NULL);
		int errcode = WSAGetLastError();
		if (err != 0) {
			//assert(errcode == WSA_IO_PENDING);
			return H_FAILED;
		}
		return H_OK;
	}
}