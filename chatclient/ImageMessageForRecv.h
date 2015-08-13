#pragma once

class SockStream;

class ImageMessageForRecv
{
public:
	ImageMessageForRecv();
	~ImageMessageForRecv();
	void setRawMessage(const std::wstring& message, const std::wstring& sender,
		const std::wstring& recver, time_t timestamp);
	std::wstring getRawMessage();
	time_t getTimeStamp();
	std::wstring getRecver();
	std::wstring getSender();
	std::vector<std::wstring> getNeedDownloadFileList(const std::wstring& imageDir);
	void writeFile(const std::wstring& imageDir, SockStream& stream);

private:
	std::wstring sender_;
	std::wstring recver_;
	time_t timestamp_;
	std::wstring rawMessage_;
	std::vector<std::wstring> fileList_;
};