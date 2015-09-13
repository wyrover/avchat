#pragma once
namespace avc
{
	class RequestFilesInfo
	{
	public:
		RequestFilesInfo();
		~RequestFilesInfo();
		std::wstring fileName;
		int fileSize;
	};
}
