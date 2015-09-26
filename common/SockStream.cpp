#include "stdafx.h"
#include "SockStream.h"
#include "buffer.h"
#include "Utils.h"
#include "StringUtils.h"

SockStream::SockStream()
{
	buff_ = nullptr;
	size_ = 0;
	curr_ = 0;
	capacity_ = 0;
	read_ = false;
}

SockStream::SockStream(char* buff, size_t len)
{
	buff_ = buff;
	size_ = len;
	curr_ = 0;
	capacity_ = size_;
	read_ = true;
}

int SockStream::writeShort(short value)
{
	return write((char*)&value, 2);
}

int SockStream::writeInt(int value)
{
	return write((char*)&value, 4);
}

int SockStream::writeInt64(int64_t value)
{
	return write((char*)&value, 8);
}

int SockStream::writeString(const std::u16string& str)
{
	int len = writeInt(str.length());
	len += write((char*)str.c_str(), str.length() * sizeof(char16_t));
	return len;
}


int SockStream::writeUtf8String(const std::string& str)
{
	writeString(StringUtils::Utf8ToUtf16String(str));
}

int SockStream::writeDouble(double value)
{
	return write((char*)&value, sizeof(double));
}

int SockStream::writeFloat(float value)
{
	return write((char*)&value, sizeof(float));
}

int SockStream::writeBool(bool value)
{
	return writeChar((char)value);
}

int SockStream::writeBoolVec(const std::vector<bool>& boolVec)
{
	int bytes = writeInt(boolVec.size());
	for (auto v : boolVec) {
		bytes += writeBool(v);
	}
	return bytes;
}

short SockStream::getShort()
{
	char* ptr = buff_ + curr_;
	short value = *(short*)ptr;
	curr_ += 2;
	return value;
}

int SockStream::getInt()
{
	if (curr_ + 4 > size_)
		throw std::out_of_range("read error");
	char* ptr = buff_ + curr_;
	int value = *(int*)ptr;
	curr_ += 4;
	return value;
}

int64_t SockStream::getInt64()
{
	if (curr_ + 8 > size_)
		throw std::out_of_range("read error");
	char* ptr = buff_ + curr_;
	int64_t value = *(int64_t*)ptr;
	curr_ += 8;
	return value;
}

std::u16string SockStream::getString()
{
	int len = getInt();
	if (curr_ + len > size_)
		throw std::out_of_range("read error");
	std::u16string result((char16_t*)(buff_ + curr_), len);
	curr_ += len * sizeof(char16_t);
	return result;
}

std::string SockStream::getUtf8String()
{
	return StringUtils::Utf16ToUtf8String(getString());
}

double SockStream::getDouble()
{
	if (curr_ + sizeof(double) > size_)
		throw std::out_of_range("read error");
	char* ptr = buff_ + curr_;
	double value = *(double*)ptr;
	curr_ += sizeof(double);
	return value;
}

float SockStream::getFloat()
{
	if (curr_ + sizeof(float) > size_)
		throw std::out_of_range("read error");
	char* ptr = buff_ + curr_;
	float value = *(float*)ptr;
	curr_ += sizeof(float);
	return value;
}

bool SockStream::getBool()
{
	if (curr_ + sizeof(char) > size_)
		throw std::out_of_range("read error");
	char* ptr = buff_ + curr_;
	bool value = !!*ptr;
	curr_ += sizeof(char);
	return value;
}

char* SockStream::getBuf()
{
	return buff_;
}

size_t SockStream::getSize()
{
	return size_;
}

void SockStream::ensureSize(int incre)
{
	if (size_ + incre > capacity_) {
		size_t newCapacity = (size_ + incre) * 2;
		char* newPtr = new char[newCapacity];
		memcpy(newPtr, buff_, size_);
		delete[] buff_;
		buff_ = newPtr;
		capacity_ = newCapacity;
	}
}

int SockStream::write(char* value, size_t len)
{
	ensureSize(len);
	memcpy(buff_ + size_, value, len);
	size_ += len;
	return len;
}

int SockStream::writeChar(char ch)
{
	write(&ch, 1);
	return 1;
}

SockStream::~SockStream()
{
	if (!read_)
		delete[] buff_;
}

void SockStream::clear()
{
	if (!read_) {
		delete[] buff_;
		buff_ = nullptr;
		size_ = 0;
		curr_ = 0;
		capacity_ = 0;
	}
}

char* SockStream::getCurrPtr()
{
	if (read_)
		return buff_ + curr_;
	else
		return buff_ + size_;
}

int SockStream::writeStringVec(const std::vector<std::u16string>& strVec)
{
	int bytes = writeInt(strVec.size());
	for (auto str : strVec) {
		bytes += writeString(str);
	}
	return bytes;
}

int SockStream::writeUtf8StringVec(const std::vector<std::string>& strVec)
{
	int bytes = writeInt(strVec.size());
	for (auto str : strVec) {
		bytes += writeUtf8String(str);
	}
	return bytes;
}

std::vector<std::u16string> SockStream::getStringVec()
{
	std::vector<std::u16string> result;
	int count = getInt();
	while (count--) {
		result.push_back(getString());
	}
	return result;
}


std::vector<std::string> SockStream::getUtf8StringVec()
{
	std::vector<std::string> result;
	int count = getInt();
	while (count--) {
		result.push_back(getUtf8String());
	}
	return result;
}

std::vector<bool> SockStream::getBoolVec()
{
	std::vector<bool> result;
	int count = getInt();
	while (count--) {
		result.push_back(getBool());
	}
	return result;
}

void SockStream::flushSize()
{
	auto pSize = (int*)(getBuf() + 4);
	*pSize = getSize();
}

int SockStream::writeBuffer(buffer& buf)
{
	int bytes = writeInt(buf.size());
	bytes += write(buf.data(), buf.size());
	return bytes;
}

void SockStream::getBuffer(buffer& buf)
{
	buffer tmpBuf;
	int bytes = getInt();
	char* ptr = buff_ + curr_;
	tmpBuf.append(ptr, bytes);
	curr_ += bytes;
	tmpBuf.swap(buf);
}

