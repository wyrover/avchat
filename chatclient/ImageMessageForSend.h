#pragma once

#include <string>
#include <vector>

namespace avc
{
	class ImageMessageForSend
	{
	public:
		ImageMessageForSend();
		~ImageMessageForSend();
		void setRawMessage(const std::u16string& message, const std::u16string& recver, time_t timestamp);
		std::u16string getRawMessage();
		time_t getTimeStamp();
		std::u16string getRecver();
		std::vector<std::u16string> getHashList();
		std::vector<std::u16string> getUploadFileList(const std::vector<std::u16string>& checkList);
		std::u16string translateMessage(const std::vector<std::u16string>& uploadList);

	private:
		std::u16string recver_;
		time_t timestamp_;
		std::u16string rawMessage_;
		std::vector<std::u16string> fileList_;
		std::vector<std::u16string> hashList_;
		std::vector<std::u16string> urlList_;
	};
}
