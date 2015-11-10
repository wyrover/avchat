#include "stdafx.h"
#include <assert.h>
#include <random>
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

std::u16string su::GenerateRandomString(int len)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<double> dist(0, 26);
	std::u16string result;
	for (int i = 0; i < len; ++i)  {
		result.push_back(u'A' + dist(mt));
	}
	return result;
}

std::u16string su::XorString(const std::u16string& str1, const std::u16string& str2)
{
	assert(str1.size() == str2.size());
	std::u16string result;
	result.resize(str1.size());
	for (int i = 0; i < str1.size(); ++i) {
		unsigned short ch1 = str1[i];
		unsigned short ch2 = str2[i];
		result[i] = ch1 ^ ch2;
	}
	return result;
}

