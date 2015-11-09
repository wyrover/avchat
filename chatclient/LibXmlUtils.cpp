#include "stdafx.h"
#include "../common/errcode.h"
#include "../common/StringUtils.h"
#include "../common/trace.h"
#include <assert.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include "LibXmlUtils.h"

namespace avc
{
	int LibXmlUtils::XmlToImageList(const std::u16string& xml, std::vector<std::u16string>* fileList)
	{
		auto aFullStr = u"<msg>" + xml + u"</msg>";
		auto uFullStr = su::u16to8(aFullStr);
		xmlDocPtr doc = xmlParseDoc((const xmlChar*)uFullStr.c_str()); 
		if (!doc) {
			return H_FAILED;
		}
		xmlNodePtr root = xmlDocGetRootElement(doc);
		if (!root) {
			return H_FAILED;
		}
		for (auto child = root->children; child; child = child->next) {
			if (child->type == XML_ELEMENT_NODE && xmlStrcmp(child->name, (const xmlChar*)"img") == 0) {
				xmlChar* src = xmlGetProp(child, (const xmlChar*)"src");
				std::u16string value;
				if (src) {
					value = su::u8to16((const char*)src);
					xmlFree(src);
				} else {
					auto path = xmlGetProp(child, (const xmlChar*)"path");
					if (path) {
						value = su::u8to16((const char*)path);
						xmlFree(path);
					} 
				}
				if (!value.empty()) {
					fileList->push_back(value);
				}
			}
		}
		xmlFreeDoc(doc);
		return H_OK;
	}


	// filepath to src
	int LibXmlUtils::XmlTranslateMessage(const std::u16string& xmlMessage, const std::map<std::u16string, std::u16string>& fileUrlMap,
			std::u16string* message)
	{
		auto aFullStr = u"<msg>" + xmlMessage + u"</msg>";
		auto uFullStr = su::u16to8(aFullStr);
		xmlDocPtr doc = xmlParseDoc((const xmlChar*)uFullStr.c_str()); 
		if (!doc) {
			return H_FAILED;
		}
		xmlNodePtr root = xmlDocGetRootElement(doc);
		if (!root) {
			return H_FAILED;
		}
		std::string uMessage;
		for (auto child = root->children; child; child = child->next) {
			if (child->type == XML_TEXT_NODE) {
				uMessage += (const char*)child->content;
			} else if (child->type == XML_ELEMENT_NODE && xmlStrcmp(child->name, (const xmlChar*)"img") == 0) {
				xmlChar* path = xmlGetProp(child, (const xmlChar*)"path");
				std::u16string value;
				if (path) {
					std::u16string wPath = su::u8to16((const char*)path);
					auto value = fileUrlMap.at(wPath);
					if (!value.empty()) {
						std::string uValue = su::u16to8(value);
						char buff[256];
						snprintf(buff, 256, "<img src=\"%s\"/>", uValue.c_str());
						uMessage += buff;
					}
				}
			}
		}
		xmlFreeDoc(doc);
		printf("umessage = %s\n", uMessage.c_str());
		*message = su::u8to16(uMessage);
		return H_OK;
	}

	// replate img with 0xfffc and fill the image list
	int LibXmlUtils::XmlMessageToQtMessage(const std::u16string& xmlMessage, std::vector<std::u16string>* imageList,
			std::u16string* message)
	{
		auto aFullStr = u"<msg>" + xmlMessage + u"</msg>";
		auto uFullStr = su::u16to8(aFullStr);
		xmlDocPtr doc = xmlParseDoc((const xmlChar*)uFullStr.c_str()); 
		if (!doc) {
			return H_FAILED;
		}
		xmlNodePtr root = xmlDocGetRootElement(doc);
		if (!root) {
			return H_FAILED;
		}

		std::string uMessage;
		for (auto child = root->children; child; child = child->next) {
			if (child->type == XML_ELEMENT_NODE && xmlStrcmp(child->name, (const xmlChar*)"img") == 0) {
				xmlChar* src = xmlGetProp(child, (const xmlChar*)"src");
				std::u16string value;
				if (src) {
					value = su::u8to16((const char*)src);
					xmlFree(src);
				} else {
					auto path = xmlGetProp(child, (const xmlChar*)"path");
					if (path) {
						value = su::u8to16((const char*)path);
						xmlFree(path);
					} 
				}
				if (!value.empty()) {
					uMessage += "\xef\xbf\xbc";
					imageList->push_back(value);
				}
			} else if (child->type == XML_TEXT_NODE) {
				uMessage += (const char*)child->content;
			}
		}
		xmlFreeDoc(doc);
		assert(!imageList->empty());
		*message = su::u8to16(uMessage);
		return H_OK;
	}
}
