#pragma once

class Utils
{
public:
	Utils();
	~Utils();
	static unsigned int GetCpuCount();
	static int GeneratePasswordHash(const std::string& password, std::string* password_hash);
	static std::string GeneratePasswordHash(const std::string& password, const std::string& salt);
	static bool ValidatePasswordHash(const std::string& password, const std::string& password_hash);
	static std::string Utf16ToUtf8String(const std::wstring& str);
	static std::wstring Utf8ToUtf16String(const std::string& str);
};

