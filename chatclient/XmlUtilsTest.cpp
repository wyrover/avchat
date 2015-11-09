#include "stdafx.h"
#include "LibXmlUtils.h"
#include "RapidXmlUtils.h"
#include <tuple>
#include <gtest/gtest.h>

TEST(XmlUtils, XmlTranslateMessage)
{
	std::u16string xmlMessage = u"abc<img path=\"c:/keke.png\"/>";
	std::map<std::u16string, std::u16string> fileMap;
	fileMap[u"c:/keke.png"] = u"abc.png";
	std::u16string msg;
	avc::LibXmlUtils::XmlTranslateMessage(xmlMessage, fileMap, &msg);
	EXPECT_EQ(msg, u"abc<img src=\"abc.png\"/>");
}

TEST(XmlUtils, xmlToImageList) 
{
	std::vector<std::pair<std::u16string, std::vector<std::u16string>>> testSet;
	std::vector<std::u16string> list1, list2;
	list1.push_back(u"c:/keke.png");
	list2.push_back(u"/Users/jcyangzh/Desktop/keke.png");
	testSet.push_back(std::make_pair(u"abc<img src=\"c:/keke.png\"/>", list1));
	testSet.push_back(std::make_pair(u"abc<img path=\"c:/keke.png\"/>", list1));
	testSet.push_back(std::make_pair(u"<img path=\"/Users/jcyangzh/Desktop/keke.png\"/>", list2));

	for (auto& item : testSet) {
		//{
			//std::vector<std::u16string> imageList;
			//avc::RapidXmlUtils::XmlToImageList(item.first, &imageList);
			//EXPECT_EQ(imageList, item.second);
		//}


		{
			std::vector<std::u16string> imageList;
			avc::LibXmlUtils::XmlToImageList(item.first, &imageList);
			EXPECT_EQ(imageList, item.second);
		}

	}
}

TEST(XmlUtils, xmlMessageToQtMessage)
{
	std::vector<std::tuple<std::u16string, std::u16string, std::vector<std::u16string>>> testSet;
	std::vector<std::u16string> list1, list2;
	list1.push_back(u"c:/keke.png");
	list2.push_back(u"/Users/jcyangzh/Desktop/keke.png");
	testSet.push_back(std::make_tuple(u"abc<img src=\"c:/keke.png\"/>", u"abc\xfffc", list1));
	testSet.push_back(std::make_tuple(u"abc<img path=\"c:/keke.png\"/>", u"abc\xfffc", list1));
	testSet.push_back(std::make_tuple(u"<img path=\"/Users/jcyangzh/Desktop/keke.png\"/>", u"\xfffc", list2));

	for (auto& item : testSet) {
		//{
			//std::vector<std::u16string> imageList;
			//std::u16string qtMessage;
			//avc::RapidXmlUtils::XmlMessageToQtMessage(std::get<0>(item), &imageList, &qtMessage);
			//EXPECT_EQ(qtMessage, std::get<1>(item));
			//EXPECT_EQ(imageList, std::get<2>(item));
		//}

		{
			std::vector<std::u16string> imageList;
			std::u16string qtMessage;
			avc::LibXmlUtils::XmlMessageToQtMessage(std::get<0>(item), &imageList, &qtMessage);
			EXPECT_EQ(qtMessage, std::get<1>(item));
			EXPECT_EQ(imageList, std::get<2>(item));
		}
	}
}

