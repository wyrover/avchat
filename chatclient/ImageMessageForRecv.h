#pragma once

class SockStream;
#include <string>
#include <vector>
namespace avc
{
	class ImageMessageForRecv
	{
	public:
		ImageMessageForRecv();
		~ImageMessageForRecv();
		void setRawMessage(const std::u16string& message, const std::u16string& sender,
			const std::u16string& recver, time_t timestamp);
		std::u16string getRawMessage();
		time_t getTimeStamp();
		std::u16string getRecver();
		std::u16string getSender();
		std::vector<std::u16string> getNeedDownloadFileList(const std::u16string& imageDir);
		void writeFile(const std::u16string& imageDir, SockStream& stream);

	private:
		std::u16string sender_;
		std::u16string recver_;
		time_t timestamp_;
		std::u16string rawMessage_;
		std::vector<std::u16string> fileList_;
	};
}
