#include "stdafx.h"
#include "StringUtils.h"

StringUtils::StringUtils()
{

}

StringUtils::~StringUtils()
{

}

std::string StringUtils::Utf16ToUtf8String(const std::wstring& str)
{
	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	return convert.to_bytes(str);
}

std::wstring StringUtils::Utf8ToUtf16String(const std::string& str)
{
	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	return convert.from_bytes(str);
}