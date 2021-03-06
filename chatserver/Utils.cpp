#include "stdafx.h"
#include "Utils.h"
#include <array>
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


static std::vector<unsigned char> kPngMagic = {
	0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a
};

static std::vector<unsigned char> kGifMagic1 = {
	0x47, 0x49, 0x46, 0x38, 0x39, 0x61
};
static std::vector<unsigned char> kGifMagic2 = {
	0x47, 0x49, 0x46, 0x38, 0x37, 0x61
};
static std::vector<unsigned char> kJpegExifMagic = {
	0xFF, 0xD8, 0xFF, 0xE1,
};
static std::vector<unsigned char> kJpegJfifMagic = {
	0xFF, 0xD8, 0xFF, 0xE0,
};
static std::vector<unsigned char> kBmpMagic = {
	0x42, 0x4d,
};

static std::vector<std::wstring> kImageExts = {
	L"jpg", L"jpeg", L"gif", L"png", L"bmp",
};

bool compareMagic(buffer& buf, const std::vector<unsigned char>& magic) {
	if (buf.size() > magic.size()) {
		bool isImage = true;
		for (int i = 0; i < magic.size(); ++i) {
			if (magic[i] != (unsigned char)buf[i]) {
				isImage = false;
				break;
			}
		}
		if (isImage)
			return true;
	}
	return false;
}

bool Utils::IsImage(buffer& buf)
{
	if (compareMagic(buf, kPngMagic))
		return true;
	if (compareMagic(buf, kGifMagic1))
		return true;
	if (compareMagic(buf, kGifMagic2))
		return true;
	if (compareMagic(buf, kJpegJfifMagic))
		return true;
	if (compareMagic(buf, kJpegExifMagic))
		return true;
	if (compareMagic(buf, kBmpMagic))
		return true;
	return false;
}


bool Utils::IsImageExt(const std::wstring& ext)
{
	return std::find_if(kImageExts.begin(), kImageExts.end(), [&ext](const std::wstring& item) {
		return boost::iequals(item, ext); }) != kImageExts.end();
}

std::wstring Utils::GetRandomFileName()
{
	using namespace CryptoPP;
	AutoSeededRandomPool rng;
	std::vector<byte> buffer(13);
	rng.GenerateBlock(buffer.data(), buffer.size());
	std::wstring fileName;
	for (int i = 0; i < buffer.size(); i++) {
		// restrict to length of range [a..z0..9]
		int b = (buffer[i] % 36);
		wchar_t c = (char)(b < 26 ? (b + L'a') : (b - 26 + L'0'));
		fileName += c;
	}
	return fileName;
}