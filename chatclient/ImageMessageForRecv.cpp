#include "stdafx.h"
#include "ImageMessageForRecv.h"
#include "../common/FileUtils.h"
#include "../common/SockStream.h"
#include "../common/StringUtils.h"
#include "Utils.h"
namespace avc
{
	ImageMessageForRecv::ImageMessageForRecv()
	{
	}


	ImageMessageForRecv::~ImageMessageForRecv()
	{
	}

	void ImageMessageForRecv::setRawMessage(const std::u16string& message, const std::u16string& sender,
		const std::u16string& recver, time_t timestamp)
	{
		rawMessage_ = message;
		sender_ = sender;
		recver_ = recver;
		timestamp_ = timestamp;
	}

	std::u16string ImageMessageForRecv::getRawMessage()
	{
		return rawMessage_;
	}

	std::vector<std::u16string> ImageMessageForRecv::getNeedDownloadFileList(const std::u16string& imageDir)
	{
		std::vector<std::u16string> needList;
		avc::Utils::XmlToImageList(rawMessage_, &fileList_);
		for (auto filePath : fileList_) {
			auto pos = filePath.rfind(u'/');
			auto filename = filePath.substr(pos, filePath.length() - pos);
			if (!FileUtils::FileExists(su::u16to8(imageDir + filename).c_str())) {
				needList.push_back(filePath);
			}
		}
		return needList;
	}

	time_t ImageMessageForRecv::getTimeStamp()
	{
		return timestamp_;
	}

	std::u16string ImageMessageForRecv::getRecver()
	{
		return recver_;
	}

	void ImageMessageForRecv::writeFile(const std::u16string& imageDir, SockStream& stream)
	{
		auto count = stream.getInt();
		for (int i = 0; i < count; ++i) {
			buffer fileBuf;
			stream.getBuffer(fileBuf);
			auto fileName = imageDir + fileList_[i];
			FileUtils::FnCreateFile(su::u16to8(fileName), fileBuf);
		}
	}

	std::u16string ImageMessageForRecv::getSender()
	{
		return sender_;
	}
}
