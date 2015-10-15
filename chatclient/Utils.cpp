#include "stdafx.h"
#include "Utils.h"
#include "../rapidxml/rapidxml.hpp"
#include "../common/errcode.h"
#include "../common/StringUtils.h"
#include "../common/trace.h"

#define LOG_XML 1

using namespace rapidxml;
namespace avc
{
	Utils::Utils()
	{
	}


	Utils::~Utils()
	{
	}


	int Utils::XmlToImageList(const std::u16string& xml, std::vector<std::u16string>* fileList)
	{
		auto aFullStr = u"<msg>" + xml + u"</msg>";
		auto uFullStr = su::u16to8(aFullStr);
		std::vector<char> buf(uFullStr.begin(), uFullStr.end());
		xml_document<> doc;
		doc.parse<0>(buf.data());
		auto root = doc.first_node();
		for (auto child = root->first_node(); child; child = child->next_sibling()) {
			if (child->type() == node_element && strcmp(child->name(), "img") == 0) {
				auto src = child->first_attribute("src");
				std::u16string value;
				if (src) {
					value = su::u8to16(src->value());
				} else {
					auto path = child->first_attribute("path");
					if (path) {
						value = su::u8to16(path->value());
					} 
				}
				if (!value.empty()) {
					fileList->push_back(value);
				}
			}
		}
		return H_OK;
	}

	
	// filepath to src
	int Utils::XmlTranslateMessage(const std::u16string& xmlMessage, const std::map<std::u16string, std::u16string>& fileUrlMap,
		std::u16string* message)
	{
		std::string uMessage;
		auto aFullStr = u"<msg>" + xmlMessage + u"</msg>";
		auto uFullStr = su::u16to8(aFullStr);
		std::vector<char> buf(uFullStr.begin(), uFullStr.end());
		xml_document<> doc;
		doc.parse<0>(buf.data());
		auto root = doc.first_node();
		for (auto child = root->first_node(); child; child = child->next_sibling()) {
			if (child->type() == node_data) {
				uMessage += child->value();
			} else if (child->type() == node_element && strcmp(child->name(), "img") == 0) {
				auto path = child->first_attribute("path");
				if (path) {
					std::u16string wPath = su::u8to16(path->value());
					auto value = fileUrlMap.at(wPath);
					if (!value.empty()) {
						std::string uValue = su::u16to8(value);
						char buff[256];
						snprintf(buff, 256, "<img src=\"%s\" />", uValue.c_str());
						uMessage += buff;
					}
				} 
			}
		}
		*message = su::u8to16(uMessage);
		return H_OK;
	}

	// replate img with 0xfffc and fill the image list
	int Utils::XmlMessageToQtMessage(const std::u16string& xmlMessage, std::vector<std::u16string>* imageList,
		std::u16string* message)
	{
		std::string uMessage;
		auto aFullStr = u"<msg>" + xmlMessage + u"</msg>";
		auto uFullStr = su::u16to8(aFullStr);
		std::vector<char> buf(uFullStr.begin(), uFullStr.end());
		xml_document<> doc;
		doc.parse<0>(buf.data());
		auto root = doc.first_node();
		for (auto child = root->first_node(); child; child = child->next_sibling()) {
			if (child->type() == node_data) {
				uMessage += child->value();
			} else if (child->type() == node_element && strcmp(child->name(), "img") == 0) {
				auto src = child->first_attribute("src");
				std::u16string value;
				if (src) {
					value = su::u8to16(src->value());
				} else {
					auto path = child->first_attribute("path");
					if (path) {
						value = su::u8to16(path->value());
					} 
				}
				if (!value.empty()) {
					uMessage + "\xef\xbf\xbc";
					imageList->push_back(value);
				}
			}
		}
		*message = su::u8to16(uMessage);
		return H_OK;
	}
}
