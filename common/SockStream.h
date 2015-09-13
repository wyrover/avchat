#pragma once
class buffer;
class SockStream
{
public:
	SockStream();
	SockStream(char* buff, size_t len);
	~SockStream();
	void clear();
	int writeShort(short value);
	int writeInt(int value);
	int writeInt64(int64_t value);
	int writeString(const std::wstring& str);
	int writeDouble(double value);
	int writeFloat(float value);
	int writeBool(bool value);
	int writeBoolVec(const std::vector<bool>& boolVec);
	int writeChar(char ch);
	int writeBuffer(buffer& buf);
	int writeStringVec(const std::vector<std::wstring>& strVec);
	void flushSize();

	short getShort();
	int getInt();
	int64_t getInt64();
	std::wstring getString();
	double getDouble();
	float getFloat();
	bool getBool();
	void getBuffer(buffer& buf);
	std::vector<std::wstring> getStringVec();
	std::vector<bool> getBoolVec();

	char* getBuf();
	char* getCurrPtr();
	size_t getSize();

private:
	int write(char* value, size_t len);
	void ensureSize(int incre);
	char* buff_;
	size_t size_;
	size_t capacity_;
	int curr_;
	bool read_;
};