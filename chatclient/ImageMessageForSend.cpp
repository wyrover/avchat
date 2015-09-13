#include "stdafx.h"
#include "ImageMessageForSend.h"
#include "../common/FileUtils.h"
#include "Utils.h"
namespace avc
{
	ImageMessageForSend::ImageMessageForSend()
	{
	}


	ImageMessageForSend::~ImageMessageForSend()
	{
	}

	void ImageMessageForSend::setRawMessage(const std::wstring& message, const std::wstring& recver, time_t timestamp)
	{
		rawMessage_ = message;
		recver_ = recver;
		timestamp_ = timestamp;
	}

	std::wstring ImageMessageForSend::getRawMessage()
	{
		return rawMessage_;
	}

	std::vector<std::wstring> ImageMessageForSend::getHashList()
	{
		avc::Utils::XmlToImageList(rawMessage_, &fileList_);
		for (auto filePath : fileList_) {
			std::wstring hash;
			FileUtils::CalculateFileSHA1(filePath, &hash);
			hashList_.push_back(hash);
		}
		return hashList_;
	}

	std::vector<std::wstring> ImageMessageForSend::getUploadFileList(const std::vector<std::wstring>& checkList)
	{
		urlList_ = checkList;
		std::vector<std::wstring> uploadList;
		for (int i = 0; i < checkList.size(); ++i) {
			if (checkList[i].empty()) {
				uploadList.push_back(fileList_[i]);
			}
		}
		return uploadList;
	}

	std::wstring ImageMessageForSend::translateMessage(const std::vector<std::wstring>& uploadList)
	{
		int pos = 0;
		for (int i = 0; i < urlList_.size(); ++i) {
			if (urlList_[i].empty()) {
				urlList_[i] = uploadList[pos];
				pos++;
			}
		}
		std::map<std::wstring, std::wstring> fileMap;
		for (int i = 0; i < urlList_.size(); ++i) {
			fileMap[fileList_[i]] = urlList_[i];
		}
		std::wstring message;
		avc::Utils::XmlTranslateMessage(rawMessage_, fileMap, &message);
		return message;
	}

	time_t ImageMessageForSend::getTimeStamp()
	{
		return timestamp_;
	}

	std::wstring ImageMessageForSend::getRecver()
	{
		return recver_;
	}
}
