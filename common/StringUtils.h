#pragma once

#include <string>

class su
{
public:
	su();
	~su();
	static std::string u16to8(const std::u16string& str);
	static std::u16string u8to16(const std::string& str);
	static std::string buf2string(unsigned char* buf,  int len); 
	static std::u16string tolower(const std::u16string& str);
	static std::string tolower(const std::string& str);
	static std::u16string GenerateRandomString(int len);
	static std::u16string XorString(const std::u16string& str1, const std::u16string& str2);
};

