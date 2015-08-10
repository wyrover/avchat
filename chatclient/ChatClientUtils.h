#pragma once

class ChatClientUtils
{
public:
	ChatClientUtils();
	~ChatClientUtils();
	static int CalculateFileSHA1(const std::wstring& filePath, std::wstring* hash);
	static int XmlToImageList(const std::wstring& xml, std::vector<std::wstring>* fileList);
	static int XmlMessageToRawMessage(const std::wstring& xmlMessage, const std::vector<std::wstring>* urlList,
		std::wstring* rawMessage);
};

