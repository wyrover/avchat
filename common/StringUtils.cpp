#include "stdafx.h"
#include <sstream>
#include <iomanip>
#include <boost/locale/encoding_utf.hpp>
#include "StringUtils.h"

su::su()
{

}

su::~su()
{

}

std::string su::u16to8(const std::u16string& str)
{
	return boost::locale::conv::utf_to_utf<char>(str);
}

std::u16string su::u8to16(const std::string& str)
{
	return boost::locale::conv::utf_to_utf<char16_t>(str);
}

std::string su::buf2string(unsigned char* buf, int len)
{
	std::stringstream sb;
	sb.setf(std::ios_base::hex, std::ios_base::basefield);
	sb.fill('0');
	for (int i = 0; i < len; ++i) {
		sb << std::setw(2) << (int)buf[i];
	}
	return sb.str();
}

std::u16string su::tolower(const std::u16string& str) 
{
	auto rc = str;
	std::transform(rc.begin(), rc.end(), rc.begin(), ::tolower);
	return rc;
}

std::string su::tolower(const std::string& str)
{
	auto rc = str;
	std::transform(rc.begin(), rc.end(), rc.begin(), ::tolower);
	return rc;
}

