#pragma once
class StringUtils
{
public:
	StringUtils();
	~StringUtils();
	static std::string Utf16ToUtf8String(const std::wstring& str);
	static std::wstring Utf8ToUtf16String(const std::string& str);
};

