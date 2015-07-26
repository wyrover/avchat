#include "stdafx.h"
#include "SockStream.h"

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
	return write((char)value);
}

int SockStream::getInt()
{
	char* ptr = buff_ + curr_;
	int value = *(int*)ptr;
	curr_ += 4;
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

int SockStream::write(char ch)
{
	write(&ch, 1);
	return 1;
}

SockStream::~SockStream()
{
	if (!read_)
		delete[] buff_;
}

char* SockStream::getCurrPtr()
{
	if (read_)
		return buff_ + curr_;
	else
		return buff_ + size_;
}
