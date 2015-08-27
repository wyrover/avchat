#include "stdafx.h"
#include "SockStream.h"
#include "buffer.h"

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

int SockStream::writeInt(int value)
{
	return write((char*)&value, 4);
}

int SockStream::writeInt64(int64_t value)
{
	return write((char*)&value, 8);
}

int SockStream::writeString(const std::wstring& str)
{
	int len = writeInt(str.length());
	len += write((char*)str.c_str(), str.length() * sizeof(wchar_t));
	return len;
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

int SockStream::getInt()
{
	char* ptr = buff_ + curr_;
	int value = *(int*)ptr;
	curr_ += 4;
	return value;
}

int64_t SockStream::getInt64()
{
	char* ptr = buff_ + curr_;
	int64_t value = *(int64_t*)ptr;
	curr_ += 8;
	return value;
}




std::wstring SockStream::getString()
{
	int len = getInt();
	std::wstring result((wchar_t*)(buff_ + curr_), len);
	curr_ += len * sizeof(wchar_t);
	return result;
}

double SockStream::getDouble()
{	
	char* ptr = buff_ + curr_;
	double value = *(double*)ptr;
	curr_ += sizeof(double);
	return value;
}

float SockStream::getFloat()
{
	char* ptr = buff_ + curr_;
	float value = *(float*)ptr;
	curr_ += sizeof(float);
	return value;
}

bool SockStream::getBool()
{
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

int SockStream::writeStringVec(const std::vector<std::wstring>& strVec)
{
	int bytes = writeInt(strVec.size());
	for (auto str : strVec) {
		bytes += writeString(str);
	}
	return bytes;
}

std::vector<std::wstring> SockStream::getStringVec()
{
	std::vector<std::wstring> result;
	int count = getInt();
	while (count--) {
		result.push_back(getString());
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


