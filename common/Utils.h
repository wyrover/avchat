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
		static HERRCODE QueueSendRequest(SOCKET socket, SockStream& stream);
		static HERRCODE QueueRecvCmdRequest(SOCKET socket);
	};
}
