#include "stdafx.h"
#include "ImageMessageForRecv.h"
#include "../common/FileUtils.h"
#include "../common/SockStream.h"
#include "Utils.h"
namespace avc
{
	ImageMessageForRecv::ImageMessageForRecv()
	{
	}


	ImageMessageForRecv::~ImageMessageForRecv()
	{
	}

	void ImageMessageForRecv::setRawMessage(const std::wstring& message, const std::wstring& sender,
		const std::wstring& recver, time_t timestamp)
	{
		rawMessage_ = message;
		sender_ = sender;
		recver_ = recver;
		timestamp_ = timestamp;
	}

	std::wstring ImageMessageForRecv::getRawMessage()
	{
		return rawMessage_;
	}

	std::vector<std::wstring> ImageMessageForRecv::getNeedDownloadFileList(const std::wstring& imageDir)
	{
		std::vector<std::wstring> needList;
		avc::Utils::XmlToImageList(rawMessage_, &fileList_);
		for (auto filePath : fileList_) {
			std::wstring filename = PathFindFileName(filePath.c_str());
			if (!FileUtils::FileExists((imageDir + filename).c_str())) {
				needList.push_back(filePath);
			}
		}
		return needList;
	}

	time_t ImageMessageForRecv::getTimeStamp()
	{
		return timestamp_;
	}

	std::wstring ImageMessageForRecv::getRecver()
	{
		return recver_;
	}

	void ImageMessageForRecv::writeFile(const std::wstring& imageDir, SockStream& stream)
	{
		auto count = stream.getInt();
		for (int i = 0; i < count; ++i) {
			buffer fileBuf;
			stream.getBuffer(fileBuf);
			auto fileName = imageDir + fileList_[i];
			FileUtils::FnCreateFile(fileName, fileBuf);
		}
	}

	std::wstring ImageMessageForRecv::getSender()
	{
		return sender_;
	}
}
