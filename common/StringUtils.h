#pragma once
#include <string>
class StringUtils
{
public:
	StringUtils();
	~StringUtils();
	static std::string Utf16ToUtf8String(const std::u16string& str);
	static std::u16string Utf8ToUtf16String(const std::string& str);
};

