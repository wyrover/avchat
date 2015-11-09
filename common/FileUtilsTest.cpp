#include "FileUtils.h"
#include <gtest/gtest.h>

TEST(FileUtils, readall)
{
	auto filename = "/Users/jcyangzh/Desktop/keke.png";
	buffer buf;
	FileUtils::ReadAll(filename, buf);
	ASSERT_TRUE(buf.size() > 0);
}

TEST(FileUtils, createFile)
{
}
