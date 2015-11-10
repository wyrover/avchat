#include "stdafx.h"
#include <thread>
#include "Utils.h"

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
		return count;
	}

}
