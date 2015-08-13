#include "stdafx.h"
#include "Utils.h"
#include "../common/errcode.h"
#include "../common/trace.h"
#include "../common/StringUtils.h"

#define LOG_XML 1
#define HR(stmt)                do { hr = (stmt); goto CleanUp; } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)

namespace {
	HRESULT GetXmlImageFilePath(IXmlReader* pReader, std::wstring* filePath)
	{
		const WCHAR* pwszPrefix;
		const WCHAR* pwszLocalName;
		const WCHAR* pwszValue;
		HRESULT hr = pReader->MoveToFirstAttribute();
		bool found = false;

		if (S_FALSE == hr)
			return hr;
		if (S_OK != hr)
		{
			TRACE_IF(LOG_XML, L"Error moving to first attribute, error is %08.8lx", hr);
			return hr;
		} else {
			while (TRUE)
			{
				if (!pReader->IsDefault())
				{
					UINT cwchPrefix;
					if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
					{
						TRACE_IF(LOG_XML, L"Error getting prefix, error is %08.8lx", hr);
						return hr;
					}
					if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
					{
						TRACE_IF(LOG_XML, L"Error getting local name, error is %08.8lx", hr);
						return hr;
					}
					if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
					{
						TRACE_IF(LOG_XML, L"Error getting value, error is %08.8lx", hr);
						return hr;
					}
					if (cwchPrefix > 0) {
						TRACE_IF(LOG_XML, L"Attr: %s:%s=\"%s\" \n", pwszPrefix, pwszLocalName, pwszValue);
					} else {
						TRACE_IF(LOG_XML, L"Attr: %s=\"%s\" \n", pwszLocalName, pwszValue);
					}
					if (wcscmp(pwszLocalName, L"path") == 0 || wcscmp(pwszLocalName, L"src") == 0) {
						*filePath = pwszValue;
						found = true;
					}
				}
				if (S_OK != pReader->MoveToNextAttribute())
					break;
			}
		}
		if (found) {
			return S_OK;
		}
		return hr;
	}
}


namespace client
{
	Utils::Utils()
	{
	}


	Utils::~Utils()
	{
	}


	int Utils::XmlToImageList(const std::wstring& xml, std::vector<std::wstring>* fileList)
	{
		HRESULT hr;
		IXmlReader *pReader = NULL;
		XmlNodeType nodeType;
		const WCHAR* pwszPrefix;
		const WCHAR* pwszLocalName;
		const WCHAR* pwszValue;
		UINT cwchPrefix;

		auto fullStr = L"<msg>" + xml + L"</msg>";
		IStream* xmlStream = SHCreateMemStream((const BYTE*)fullStr.data(), fullStr.size() * 2);
		if (!xmlStream)
			return H_FAILED;
		if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**)&pReader, NULL)))
		{
			TRACE_IF(LOG_XML, L"Error creating xml reader, error is %08.8lx", hr);
			HR(hr);
		}

		if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
		{
			TRACE_IF(LOG_XML, L"Error setting XmlReaderProperty_DtdProcessing, error is %08.8lx", hr);
			HR(hr);
		}

		if (FAILED(hr = pReader->SetInput(xmlStream)))
		{
			TRACE_IF(LOG_XML, L"Error setting input for reader, error is %08.8lx", hr);
			HR(hr);
		}

		//read until there are no more nodes
		while (S_OK == (hr = pReader->Read(&nodeType)))
		{
			switch (nodeType)
			{
				case XmlNodeType_Element:
					if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
					{
						TRACE_IF(LOG_XML, L"Error getting prefix, error is %08.8lx", hr);
						HR(hr);
					}
					if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
					{
						TRACE_IF(LOG_XML, L"Error getting local name, error is %08.8lx", hr);
						HR(hr);
					}
					if (cwchPrefix > 0)  {
						TRACE_IF(LOG_XML, L"Element: %s:%s\n", pwszPrefix, pwszLocalName);
					} else {
						TRACE_IF(LOG_XML, L"Element: %s\n", pwszLocalName);
					}

					std::wstring filePath;
					if (FAILED(hr = GetXmlImageFilePath(pReader, &filePath)))
					{
						TRACE_IF(LOG_XML, L"Error writing attributes, error is %08.8lx", hr);
						HR(hr);
					} else {
						if (SUCCEEDED(hr) && !filePath.empty())
							fileList->push_back(filePath);
					}

					if (pReader->IsEmptyElement())
						TRACE_IF(LOG_XML, L" (empty)");
					break;
			}
		}

	CleanUp:
		SAFE_RELEASE(xmlStream);
		SAFE_RELEASE(pReader);
		if (hr != S_OK)
			return H_FAILED;
		return H_OK;
	}

	
	int Utils::XmlTranslateMessage(const std::wstring& xmlMessage, const std::map<std::wstring, std::wstring>& fileUrlMap,
		std::wstring* message)
	{
		HRESULT hr;
		IXmlReader *pReader = NULL;
		IXmlWriter *pWriter = NULL;
		XmlNodeType nodeType;
		const WCHAR* pwszPrefix;
		const WCHAR* pwszLocalName;
		const WCHAR* pwszValue;
		UINT cwchPrefix;

		auto fullStr = L"<msg>" + xmlMessage + L"</msg>";
		IStream* inStream = SHCreateMemStream((const BYTE*)fullStr.data(), fullStr.size() * 2);
		if (!inStream)
			return H_FAILED;
		IStream* outStream = SHCreateMemStream(NULL, NULL);
		if (!outStream)
			return H_FAILED;
		if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**)&pReader, NULL)))
		{
			TRACE_IF(LOG_XML, L"Error creating xml reader, error is %08.8lx", hr);
			HR(hr);
		}
		if (FAILED(hr = CreateXmlWriter(__uuidof(IXmlWriter), (void**)&pWriter, NULL)))
		{
			TRACE_IF(LOG_XML, L"Error creating xml reader, error is %08.8lx", hr);
			HR(hr);
		}

		if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
		{
			TRACE_IF(LOG_XML, L"Error setting XmlReaderProperty_DtdProcessing, error is %08.8lx", hr);
			HR(hr);
		}
		if (FAILED(hr = pWriter->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
		{
			TRACE_IF(LOG_XML, L"Error setting XmlReaderProperty_DtdProcessing, error is %08.8lx", hr);
			HR(hr);
		}

		if (FAILED(hr = pReader->SetInput(inStream)))
		{
			TRACE_IF(LOG_XML, L"Error setting input for reader, error is %08.8lx", hr);
			HR(hr);
		}

		if (FAILED(hr = pWriter->SetOutput(outStream)))
		{
			TRACE_IF(LOG_XML, L"Error setting input for reader, error is %08.8lx", hr);
			HR(hr);
		}

		//read until there are no more nodes
		while (S_OK == (hr = pReader->Read(&nodeType)))
		{
			switch (nodeType)
			{
				case XmlNodeType_EndElement:
					if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
					{
						TRACE_IF(LOG_XML, L"Error getting local name, error is %08.8lx", hr);
						HR(hr);
					}
					if (wcscmp(pwszLocalName, L"img") == 0) {
						pWriter->WriteEndElement();
					}
					break;
				case XmlNodeType_Element:
				{
					if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
					{
						TRACE_IF(LOG_XML, L"Error getting prefix, error is %08.8lx", hr);
						HR(hr);
					}
					if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
					{
						TRACE_IF(LOG_XML, L"Error getting local name, error is %08.8lx", hr);
						HR(hr);
					}
					if (cwchPrefix > 0)  {
						TRACE_IF(LOG_XML, L"Element: %s:%s\n", pwszPrefix, pwszLocalName);
					} else {
						TRACE_IF(LOG_XML, L"Element: %s\n", pwszLocalName);
					}
					std::wstring filePath;
					if (FAILED(hr = GetXmlImageFilePath(pReader, &filePath)))
					{
						TRACE_IF(LOG_XML, L"Error writing attributes, error is %08.8lx", hr);
						HR(hr);
					} else {

					}
					if (pReader->IsEmptyElement())
						TRACE_IF(LOG_XML, L" (empty)");
					if (!filePath.empty()) {
						pWriter->WriteStartElement(pwszPrefix, pwszLocalName, NULL);
						auto str = fileUrlMap.at(filePath);
						pWriter->WriteAttributeString(NULL, L"src", NULL, str.c_str());
					} else {
						//pWriter->WriteAttributes(pReader, FALSE);
					}
				}
				break;
				case XmlNodeType_Text:
				case XmlNodeType_Whitespace:
					if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
					{
						TRACE_IF(LOG_XML, L"Error getting value, error is %08.8lx", hr);
						HR(hr);
					}
					if (nodeType == XmlNodeType_Text)
						pWriter->WriteString(pwszValue);
					else if (nodeType == XmlNodeType_Whitespace)
						pWriter->WriteWhitespace(pwszValue);
					break;
			}
		}
	CleanUp:
		pWriter->Flush();
		SAFE_RELEASE(pReader);
		SAFE_RELEASE(pWriter);

		STATSTG stat;
		ULONG cb = 0;
		hr = outStream->Stat(&stat, STATFLAG_DEFAULT);
		if (SUCCEEDED(hr)) {
			cb = stat.cbSize.QuadPart;
		}
		std::vector<char> v;
		v.resize(cb);
		ULONG ret;
		LARGE_INTEGER li;
		li.QuadPart = 0;
		outStream->Seek(li, STREAM_SEEK_SET, NULL);
		hr = outStream->Read(v.data(), cb, &ret);
		std::string str(v.data(), ret);
		*message = StringUtils::Utf8ToUtf16String(str);
		SAFE_RELEASE(inStream);
		SAFE_RELEASE(outStream);
		if (hr != S_OK)
			return H_FAILED;
		return H_OK;
	}

	int Utils::XmlMessageToQtMessage(const std::wstring& xmlMessage, std::vector<std::wstring>* imageList,
		std::wstring* message)
	{
		HRESULT hr;
		IXmlReader *pReader = NULL;
		IXmlWriter *pWriter = NULL;
		XmlNodeType nodeType;
		const WCHAR* pwszPrefix;
		const WCHAR* pwszLocalName;
		const WCHAR* pwszValue;
		UINT cwchPrefix;

		auto fullStr = L"<msg>" + xmlMessage + L"</msg>";
		IStream* inStream = SHCreateMemStream((const BYTE*)fullStr.data(), fullStr.size() * 2);
		if (!inStream)
			return H_FAILED;
		IStream* outStream = SHCreateMemStream(NULL, NULL);
		if (!outStream)
			return H_FAILED;
		if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**)&pReader, NULL)))
		{
			TRACE_IF(LOG_XML, L"Error creating xml reader, error is %08.8lx", hr);
			HR(hr);
		}
		if (FAILED(hr = CreateXmlWriter(__uuidof(IXmlWriter), (void**)&pWriter, NULL)))
		{
			TRACE_IF(LOG_XML, L"Error creating xml reader, error is %08.8lx", hr);
			HR(hr);
		}

		if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
		{
			TRACE_IF(LOG_XML, L"Error setting XmlReaderProperty_DtdProcessing, error is %08.8lx", hr);
			HR(hr);
		}
		if (FAILED(hr = pWriter->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
		{
			TRACE_IF(LOG_XML, L"Error setting XmlReaderProperty_DtdProcessing, error is %08.8lx", hr);
			HR(hr);
		}

		if (FAILED(hr = pReader->SetInput(inStream)))
		{
			TRACE_IF(LOG_XML, L"Error setting input for reader, error is %08.8lx", hr);
			HR(hr);
		}

		if (FAILED(hr = pWriter->SetOutput(outStream)))
		{
			TRACE_IF(LOG_XML, L"Error setting input for reader, error is %08.8lx", hr);
			HR(hr);
		}

		//read until there are no more nodes
		while (S_OK == (hr = pReader->Read(&nodeType)))
		{
			switch (nodeType)
			{
				case XmlNodeType_Element:
				{
					if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
					{
						TRACE_IF(LOG_XML, L"Error getting prefix, error is %08.8lx", hr);
						HR(hr);
					}
					if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
					{
						TRACE_IF(LOG_XML, L"Error getting local name, error is %08.8lx", hr);
						HR(hr);
					}
					if (cwchPrefix > 0)  {
						TRACE_IF(LOG_XML, L"Element: %s:%s\n", pwszPrefix, pwszLocalName);
					} else {
						TRACE_IF(LOG_XML, L"Element: %s\n", pwszLocalName);
					}
					std::wstring filePath;
					if (FAILED(hr = GetXmlImageFilePath(pReader, &filePath)))
					{
						TRACE_IF(LOG_XML, L"Error writing attributes, error is %08.8lx", hr);
						HR(hr);
					} else {

					}
					if (pReader->IsEmptyElement())
						TRACE_IF(LOG_XML, L" (empty)");
					if (!filePath.empty()) {
						imageList->push_back(filePath);
						hr = pWriter->WriteString(L"\uffec");
					} else {
						//pWriter->WriteAttributes(pReader, FALSE);
					}
				}
				break;
				case XmlNodeType_Text:
				case XmlNodeType_Whitespace:
					if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
					{
						TRACE_IF(LOG_XML, L"Error getting value, error is %08.8lx", hr);
						HR(hr);
					}
					if (nodeType == XmlNodeType_Text)
						pWriter->WriteString(pwszValue);
					else if (nodeType == XmlNodeType_Whitespace)
						pWriter->WriteWhitespace(pwszValue);
					break;
			}
		}
	CleanUp:
		pWriter->Flush();
		SAFE_RELEASE(pReader);
		SAFE_RELEASE(pWriter);

		STATSTG stat;
		ULONG cb = 0;
		hr = outStream->Stat(&stat, STATFLAG_DEFAULT);
		if (SUCCEEDED(hr)) {
			cb = stat.cbSize.QuadPart;
		}
		std::vector<char> v;
		v.resize(cb);
		ULONG ret;
		LARGE_INTEGER li;
		li.QuadPart = 0;
		outStream->Seek(li, STREAM_SEEK_SET, NULL);
		hr = outStream->Read(v.data(), cb, &ret);
		std::string str(v.data(), ret);
		*message = StringUtils::Utf8ToUtf16String(str);
		SAFE_RELEASE(inStream);
		SAFE_RELEASE(outStream);
		if (hr != S_OK)
			return H_FAILED;
		return H_OK;
	}
}
