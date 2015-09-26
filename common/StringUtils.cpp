#include "stdafx.h"
#include "StringUtils.h"
#include <boost/locale/encoding_utf.hpp>

StringUtils::StringUtils()
{

}

StringUtils::~StringUtils()
{

}

std::string StringUtils::Utf16ToUtf8String(const std::u16string& str)
{
	boost::locale::conv::utf_to_utf<char>(str.c_str(), str.c_str() + str.length());
}

std::u16string StringUtils::Utf8ToUtf16String(const std::string& str)
{
	boost::locale::conv::utf_to_utf<char16_t>(str.c_str(), str.c_str() + str.length());
}
