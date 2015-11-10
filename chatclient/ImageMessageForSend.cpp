#include "stdafx.h"
#include "ImageMessageForSend.h"
#include "../common/FileUtils.h"
#include "../common/StringUtils.h"
#include <syslog.h>
#include "XmlUtils.h"
namespace avc
{
	ImageMessageForSend::ImageMessageForSend()
	{
		timestamp_ = -1;
	}


	ImageMessageForSend::~ImageMessageForSend()
	{
	}

	void ImageMessageForSend::setRawMessage(const std::u16string& message, const std::u16string& recver, time_t timestamp)
	{
		rawMessage_ = message;
		recver_ = recver;
		timestamp_ = timestamp;
	}

	std::u16string ImageMessageForSend::getRawMessage()
	{
		return rawMessage_;
	}

	std::vector<std::u16string> ImageMessageForSend::getHashList()
	{
		avc::XmlUtils::XmlToImageList(rawMessage_, &fileList_);
		for (auto filePath : fileList_) {
			std::string hash;
			base::FileUtils::CalculateFileSHA1(su::u16to8(filePath), &hash);
			hashList_.push_back(su::u8to16(hash));
		}
		return hashList_;
	}

	std::vector<std::u16string> ImageMessageForSend::getUploadFileList(const std::vector<std::u16string>& checkList)
	{
		urlList_ = checkList;
		std::vector<std::u16string> uploadList;
		for (int i = 0; i < checkList.size(); ++i) {
			if (checkList[i].empty()) {
				uploadList.push_back(fileList_[i]);
			}
		}
		return uploadList;
	}

	std::u16string ImageMessageForSend::translateMessage(const std::vector<std::u16string>& uploadList)
	{
		int pos = 0;
		for (int i = 0; i < urlList_.size(); ++i) {
			if (urlList_[i].empty()) {
				urlList_[i] = uploadList[pos];
				pos++;
			}
		}
		std::map<std::u16string, std::u16string> fileMap;
		for (int i = 0; i < urlList_.size(); ++i) {
			fileMap[fileList_[i]] = urlList_[i];
		}
		std::u16string message;
		syslog(LOG_INFO, "rawmeessage = %s\n", su::u16to8(rawMessage_).c_str());
		avc::XmlUtils::XmlTranslateMessage(rawMessage_, fileMap, &message);
		return message;
	}

	time_t ImageMessageForSend::getTimeStamp()
	{
		return timestamp_;
	}

	std::u16string ImageMessageForSend::getRecver()
	{
		return recver_;
	}
}
