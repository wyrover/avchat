#include "stdafx.h"
#include "../common/errcode.h"
#include "../common/StringUtils.h"
#include "../common/trace.h"

#include "XmlUtils.h"
#include "RapidXmlUtils.h"
#include "LibXmlUtils.h"

#define USE_RAPID_XML 0

namespace avc
{
	XmlUtils::XmlUtils()
	{
	}


	XmlUtils::~XmlUtils()
	{
	}


	int XmlUtils::XmlToImageList(const std::u16string& xml, std::vector<std::u16string>* fileList)
	{
#if USE_RAPID_XML	
		return RapidXmlUtils::XmlToImageList(xml, fileList);
#else
		return LibXmlUtils::XmlToImageList(xml, fileList);
#endif
	}


	// filepath to src
	int XmlUtils::XmlTranslateMessage(const std::u16string& xmlMessage, const std::map<std::u16string, std::u16string>& fileUrlMap,
			std::u16string* message)
	{
#if USE_RAPID_XML
		return RapidXmlUtils::XmlTranslateMessage(xmlMessage, fileUrlMap, message);
#else
		return LibXmlUtils::XmlTranslateMessage(xmlMessage, fileUrlMap, message);
#endif
	}

	// replate img with 0xfffc and fill the image list
	int XmlUtils::XmlMessageToQtMessage(const std::u16string& xmlMessage, std::vector<std::u16string>* imageList,
			std::u16string* message)
	{
#if USE_RAPID_XML
		return RapidXmlUtils::XmlMessageToQtMessage(xmlMessage, imageList, message);
#else
		return LibXmlUtils::XmlMessageToQtMessage(xmlMessage, imageList, message);
#endif
	}
}
