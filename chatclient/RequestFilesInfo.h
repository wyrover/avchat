#pragma once
namespace avc
{
	class RequestFilesInfo
	{
	public:
		RequestFilesInfo();
		~RequestFilesInfo();
		std::u16string fileName;
		int fileSize;
	};
}
