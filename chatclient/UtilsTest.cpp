#include "stdafx.h"
#include "UtilsTest.h"
#include "Utils.h"

UtilsTest::UtilsTest()
{
}


UtilsTest::~UtilsTest()
{
}

void UtilsTest::testTranslate()
{
	std::wstring xmlMessage = L"abc<img path=\"c:/keke.png\"/>";
	std::map<std::wstring, std::wstring> fileMap;
	fileMap[L"c:/keke.png"] = L"abc.png";
	std::wstring msg;
	avc::Utils::XmlTranslateMessage(xmlMessage, fileMap, &msg);
	assert(msg == L"abc<img src=\"abc.png\"/>");
}

void UtilsTest::testTranslate2()
{
	std::wstring xmlMessage = L"abc<img path=\"keke.png\"/>";
	std::vector<std::wstring> imageList;
	std::wstring message;
	avc::Utils::XmlMessageToQtMessage(xmlMessage, &imageList, &message);
}
