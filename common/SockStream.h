#pragma once

class SockStream
{
public:
	SockStream();
	SockStream(char* buff, size_t len);
	~SockStream();
	int writeInt(int value);
	int writeString(const std::wstring& str);
	int writeDouble(double value);
	int writeFloat(float value);
	int writeBool(bool value);
	int write(char* value, size_t len);
	int write(char ch);

	int getInt();
	std::wstring getString();
	double getDouble();
	float getFloat();
	bool getBool();

	char* getBuf();
	char* getCurrPtr();
	size_t getSize();

private:
	void ensureSize(int incre);
	char* buff_;
	size_t size_;
	size_t capacity_;
	int curr_;
	bool read_;
};