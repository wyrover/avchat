#include "stdafx.h"
#include "Utils.h"

#define _CRT_SECURE_NO_DEPRECATE
#define CRYPTOPP_DEFAULT_NO_DLL
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "dll.h"
#include "md5.h"
#include "hmac.h"
#include "ripemd.h"
#include "pwdbased.h"
#include "rng.h"
#include "gzip.h"
#include "default.h"
#include "randpool.h"
#include "ida.h"
#include "base64.h"
#include "socketft.h"
#include "wait.h"
#include "factory.h"
#include "whrlpool.h"
#include "tiger.h"
#include "validate.h"
#include "bench.h"

static const int kIterations = 1000;

Utils::Utils()
{
}

Utils::~Utils()
{
}

int Utils::GeneratePasswordHash(const std::string& password, std::string* password_hash)
{
	using namespace CryptoPP;
	using namespace std;

	AutoSeededRandomPool rng;

	SecByteBlock pwsalt(SHA256::DIGESTSIZE);
	rng.GenerateBlock(pwsalt, pwsalt.size());
	SecByteBlock derivedkey(SHA256::DIGESTSIZE);

	PKCS5_PBKDF2_HMAC<SHA256> pbkdf;

	pbkdf.DeriveKey(
		derivedkey, derivedkey.size(),
		0x00,
		(byte *)password.data(), password.size(),
		pwsalt, pwsalt.size(),
		kIterations
		);
	std::string salthex;
	StringSource ss1(pwsalt, pwsalt.size(), true,
		new HexEncoder(
		new StringSink(salthex)
		)
		);
	std::string derivedhex;
	StringSource ss2(derivedkey, derivedkey.size(), true,
		new HexEncoder(
		new StringSink(derivedhex)
		)
		);
	*password_hash = salthex + "@" + derivedhex;
	return 0;
}

std::string Utils::GeneratePasswordHash(const std::string& password, const std::string& hexsalt)
{
	using namespace CryptoPP;
	using namespace std;

	AutoSeededRandomPool rng;
	SecByteBlock pwsalt(SHA256::DIGESTSIZE);
	SecByteBlock derivedkey(SHA256::DIGESTSIZE);
	PKCS5_PBKDF2_HMAC<SHA256> pbkdf;
	StringSource saltDecoder(hexsalt, true, new HexDecoder(new ArraySink(pwsalt, pwsalt.size())));
	pbkdf.DeriveKey(
		derivedkey, derivedkey.size(),
		0x00,
		(byte *)password.data(), password.size(),
		pwsalt, pwsalt.size(),
		kIterations
		);
	std::string derivedhex;
	StringSource ss2(derivedkey, derivedkey.size(), true,
		new HexEncoder(
		new StringSink(derivedhex)
		)
		);
	return derivedhex;
}

bool Utils::ValidatePasswordHash(const std::string& password, const std::string& password_hash)
{
	using namespace CryptoPP;
	using namespace std;

	AutoSeededRandomPool rng;
	SecByteBlock pwsalt(SHA256::DIGESTSIZE);
	SecByteBlock derivedkey(SHA256::DIGESTSIZE);
	PKCS5_PBKDF2_HMAC<SHA256> pbkdf;
	auto hexsalt = password_hash.substr(0, password_hash.find(L'@'));
	StringSource saltDecoder(hexsalt, true, new HexDecoder(new ArraySink(pwsalt, pwsalt.size())));
	pbkdf.DeriveKey(
		derivedkey, derivedkey.size(),
		0x00,
		(byte *)password.data(), password.size(),
		pwsalt, pwsalt.size(),
		kIterations
		);
	std::string salthex;
	StringSource ss1(pwsalt, pwsalt.size(), true,
		new HexEncoder(
		new StringSink(salthex)
		)
		);
	std::string derivedhex;
	StringSource ss2(derivedkey, derivedkey.size(), true,
		new HexEncoder(
		new StringSink(derivedhex)
		)
		);
	auto pwd_hash = salthex + "@" + derivedhex;
	return boost::iequals(pwd_hash,password_hash);
}

std::string Utils::Utf16ToUtf8String(const std::wstring& str)
{
	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	return convert.to_bytes(str);
}

std::wstring Utils::Utf8ToUtf16String(const std::string& str)
{
	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	return convert.from_bytes(str);
}