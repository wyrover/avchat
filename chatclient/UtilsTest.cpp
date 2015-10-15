#include "stdafx.h"
#include <assert.h>
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
	std::u16string xmlMessage = u"abc<img path=\"c:/keke.png\"/>";
	std::map<std::u16string, std::u16string> fileMap;
	fileMap[u"c:/keke.png"] = u"abc.png";
	std::u16string msg;
	avc::Utils::XmlTranslateMessage(xmlMessage, fileMap, &msg);
	assert(msg == u"abc<img src=\"abc.png\"/>");
}

void UtilsTest::testTranslate2()
{
	std::u16string xmlMessage = u"abc<img path=\"keke.png\"/>";
	std::vector<std::u16string> imageList;
	std::u16string message;
	avc::Utils::XmlMessageToQtMessage(xmlMessage, &imageList, &message);
}
