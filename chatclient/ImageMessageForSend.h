#pragma once

class ImageMessageForSend
{
public:
	ImageMessageForSend();
	~ImageMessageForSend();
	void setRawMessage(const std::wstring& message, const std::wstring& recver, time_t timestamp);
	std::wstring getRawMessage();
	time_t getTimeStamp();
	std::wstring getRecver();
	std::vector<std::wstring> getHashList();
	std::vector<std::wstring> getUploadFileList(const std::vector<std::wstring>& checkList);
	std::wstring translateMessage(const std::vector<std::wstring>& uploadList);

private:
	std::wstring recver_;
	time_t timestamp_;
	std::wstring rawMessage_;
	std::vector<std::wstring> fileList_;
	std::vector<std::wstring> hashList_;
	std::vector<std::wstring> urlList_;
};