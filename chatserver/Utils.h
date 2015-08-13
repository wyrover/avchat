#pragma once
class Utils
{
public:
	Utils();
	~Utils();
	static int GeneratePasswordHash(const std::string& password, std::string* password_hash);
	static std::string GeneratePasswordHash(const std::string& password, const std::string& salt);
	static bool ValidatePasswordHash(const std::string& password, const std::string& password_hash);
	static bool IsImage(buffer& buf);
	static bool IsImageExt(const std::wstring& ext);
	static std::wstring GetRandomFileName();
};

