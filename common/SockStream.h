#pragma once

class SockStream
{
public:
	SockStream();
	SockStream(char* buff, size_t len);
	~SockStream();
	int writeInt(int value);
	int writeInt64(int64_t value);
	int writeString(const std::wstring& str);
	int writeDouble(double value);
	int writeFloat(float value);
	int writeBool(bool value);
	int write(char* value, size_t len);
	int write(char ch);
	int writeStringVec(const std::vector<std::wstring>& strVec);
	void flushSize();

	int getInt();
	int64_t getInt64();
	std::wstring getString();
	double getDouble();
	float getFloat();
	bool getBool();
	std::vector<std::wstring> getStringVec();

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