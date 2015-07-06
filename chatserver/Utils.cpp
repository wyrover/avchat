#include "stdafx.h"
#include "Utils.h"

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