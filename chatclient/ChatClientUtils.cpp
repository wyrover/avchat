#include "stdafx.h"
#include "ChatClientUtils.h"
#include "../common/errcode.h"
#include "../common/trace.h"

#define LOG_XML 0
#define CHKHR(stmt)             do { hr = (stmt); if (FAILED(hr)) goto CleanUp; } while(0)
#define HR(stmt)                do { hr = (stmt); goto CleanUp; } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)

namespace {
	//-----------------------------------------------------------------------------
	// Checks if input NTSTATUS corresponds to a success code.
	//-----------------------------------------------------------------------------
	inline bool NtSuccess(NTSTATUS status)
	{
		return (status >= 0);
	}

	//-----------------------------------------------------------------------------
	// Class defining private copy constructor and operator=, to ban copies.
	//-----------------------------------------------------------------------------
	class NonCopyable
	{
	protected:
		NonCopyable() {}
		~NonCopyable() {}
	private:
		NonCopyable(const NonCopyable&);
		const NonCopyable& operator=(const NonCopyable&);
	};



	//-----------------------------------------------------------------------------
	// Error occurred during cryptographic processing.
	//-----------------------------------------------------------------------------
	class CryptException :
		public std::runtime_error
	{
	public:

		// Constructs the exception with an error message and an error code.
		explicit CryptException(const std::string & message, NTSTATUS errorCode)
			: std::runtime_error(FormatErrorMessage(message, errorCode)),
			m_errorCode(errorCode)
		{}


		// Returns the error code.
		NTSTATUS ErrorCode() const
		{
			return m_errorCode;
		}


		//
		// IMPLEMENTATION
		//
	private:
		// Error code from Cryptography API
		NTSTATUS m_errorCode;

		// Helper method to format an error message including the error code.
		static std::string FormatErrorMessage(const std::string & message, NTSTATUS errorCode)
		{
			std::ostringstream os;
			os << message << " (NTSTATUS=0x" << std::hex << errorCode << ")";
			return os.str();
		}
	};



	//-----------------------------------------------------------------------------
	// RAII wrapper to crypt algorithm provider
	//-----------------------------------------------------------------------------
	class CryptAlgorithmProvider : NonCopyable
	{
	public:

		// Creates a crypt algorithm provider object.
		// This can be used to create one ore more hash objects to hash some data.
		CryptAlgorithmProvider()
		{
			NTSTATUS result = ::BCryptOpenAlgorithmProvider(
				&m_alg,                     // algorithm handle
				BCRYPT_SHA1_ALGORITHM,      // hashing algorithm ID
				nullptr,                    // use default provider
				0                           // default flags
				);
			if (!NtSuccess(result))
				throw CryptException("Can't load default cryptographic algorithm provider.", result);
		}


		// Releases resources
		~CryptAlgorithmProvider()
		{
			::BCryptCloseAlgorithmProvider(m_alg, 0);
		}


		// Gets raw handle
		BCRYPT_ALG_HANDLE Handle() const
		{
			return m_alg;
		}


		// Gets the value of a DWORD named property
		DWORD GetDWordProperty(const std::wstring & propertyName) const
		{
			DWORD propertyValue;
			DWORD resultSize;

			//
			// Get the value of the input named property
			//

			NTSTATUS result = ::BCryptGetProperty(
				Handle(),
				propertyName.c_str(),
				reinterpret_cast<BYTE *>(&propertyValue),
				sizeof(propertyValue),
				&resultSize,
				0);
			if (!NtSuccess(result))
				throw CryptException("Can't get crypt property value.", result);

			return propertyValue;
		}


		//
		// IMPLEMENTATION
		//
	private:
		// Handle to crypt algorithm provider
		BCRYPT_ALG_HANDLE m_alg;
	};



	//-----------------------------------------------------------------------------
	// Crypt Hash object, used to hash data.
	//-----------------------------------------------------------------------------
	class CryptHashObject : NonCopyable
	{
	public:

		// Creates a crypt hash object.
		explicit CryptHashObject(const CryptAlgorithmProvider & provider)
			: m_hashObj(provider.GetDWordProperty(BCRYPT_OBJECT_LENGTH))
		{
			// Create the hash object
			NTSTATUS result = ::BCryptCreateHash(
				provider.Handle(),  // handle to parent
				&m_hash,            // hash object handle
				m_hashObj.data(),   // hash object buffer pointer
				m_hashObj.size(),   // hash object buffer length
				nullptr,            // no secret
				0,                  // no secret
				0                   // no flags
				);
			if (!NtSuccess(result))
				throw CryptException("Can't create crypt hash object.", result);
		}


		// Releases resources
		~CryptHashObject()
		{
			::BCryptDestroyHash(m_hash);
		}


		// Hashes the data in the input buffer.
		// Can be called one or more times.
		// When finished with input data, call FinishHash().
		// This method can't be called after FinisHash() is called.
		void HashData(const void * data, unsigned long length) const
		{
			// Hash this chunk of data
			NTSTATUS result = ::BCryptHashData(
				m_hash, // hash object handle
				static_cast<UCHAR *>(const_cast<void *>(data)),    // safely remove const from buffer pointer
				length, // input buffer length in bytes
				0       // no flags
				);
			if (!NtSuccess(result))
				throw CryptException("Can't hash data.", result);
		}


		// Finalizes hash calculation.
		// After this method is called, hash value can be got using HashValue() method.
		// After this method is called, the HashData() method can't be called anymore.
		void FinishHash()
		{
			//
			// Retrieve the hash of the accumulated data
			//

			BYTE hashValue[20]; // SHA-1: 20 bytes = 160 bits

			NTSTATUS result = ::BCryptFinishHash(
				m_hash,             // handle to hash object
				hashValue,          // output buffer for hash value
				sizeof(hashValue),  // size of this buffer
				0                   // no flags
				);
			if (!NtSuccess(result))
				throw CryptException("Can't finalize hashing.", result);


			//
			// Get the hash digest string from hash value buffer.
			//

			// Each byte --> 2 hex chars
			m_hashDigest.resize(sizeof(hashValue) * 2);

			// Upper-case hex digits
			static const wchar_t hexDigits[] = L"0123456789ABCDEF";

			// Index to current character in destination string
			size_t currChar = 0;

			// For each byte in the hash value buffer
			for (size_t i = 0; i < sizeof(hashValue); ++i)
			{
				// high nibble
				m_hashDigest[currChar] = hexDigits[(hashValue[i] & 0xF0) >> 4];
				++currChar;

				// low nibble
				m_hashDigest[currChar] = hexDigits[(hashValue[i] & 0x0F)];
				++currChar;
			}
		}


		// Gets the hash value (in hex, upper-case).
		// Call this method *after* FinishHash(), not before.
		// (If called before, an empty string is returned.)
		std::wstring HashDigest() const
		{
			return m_hashDigest;
		}


		//
		// IMPLEMENTATION
		//
	private:

		// Handle to hash object
		BCRYPT_HASH_HANDLE m_hash;

		// Buffer to store hash object
		std::vector<BYTE> m_hashObj;

		// Hash digest string (hex)
		std::wstring m_hashDigest;
	};



	//-----------------------------------------------------------------------------
	// Wrapper around C FILE *, for reading binary data from file.
	//-----------------------------------------------------------------------------
	class FileReader : NonCopyable
	{
	public:

		// Opens the specified file.
		explicit FileReader(const std::wstring & filename)
		{
			if (_wfopen_s(&m_file, filename.c_str(), L"rb") != 0)
			{
				throw std::runtime_error("Can't open file for reading.");
			}
		}


		// Closes the file.
		~FileReader()
		{
			if (m_file != nullptr)
				fclose(m_file);
		}


		// End Of File reached?
		bool EoF() const
		{
			return feof(m_file) ? true : false;
		}


		// Reads bytes from file to a memory buffer.
		// Returns the number of bytes actually read.
		int Read(void * buffer, int bufferSize)
		{
			return fread(buffer, 1, bufferSize, m_file);
		}


		//
		// IMPLEMENTATION
		//
	private:
		// Raw C file handle
		FILE * m_file;
	};



	//-----------------------------------------------------------------------------
	// Hashes a file with SHA-1.
	// Returns the hash digest, in hex.
	//-----------------------------------------------------------------------------
	std::wstring HashFileSHA1(const std::wstring & filename)
	{
		// Create the algorithm provider for SHA-1 hashing
		CryptAlgorithmProvider sha1;

		// Create the hash object for the particular hashing
		CryptHashObject hasher(sha1);

		// Object to read data from file
		FileReader file(filename);

		// Read buffer
		std::vector<BYTE> buffer(4 * 1024);   // 4 KB buffer

		// Reading loop
		while (!file.EoF())
		{
			// Read a chunk of data from file to memory buffer
			int readBytes = file.Read(buffer.data(), buffer.size());

			// Hash this chunk of data
			hasher.HashData(buffer.data(), readBytes);
		}

		// Finalize hashing
		hasher.FinishHash();

		// Return hash digest
		return hasher.HashDigest();
	}

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
					if (wcscmp(pwszLocalName, L"path") == 0) {
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

ChatClientUtils::ChatClientUtils()
{
}


ChatClientUtils::~ChatClientUtils()
{
}

int ChatClientUtils::CalculateFileSHA1(const std::wstring& filePath, std::wstring* pHash)
{
	try {
		std::wstring hash = HashFileSHA1(filePath);
		*pHash = hash;
		return 0;
	} catch (const std::exception& e) {
		return -1;
	}
}



int ChatClientUtils::XmlToImageList(const std::wstring& xml, std::vector<std::wstring>* fileList)
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

int ChatClientUtils::XmlMessageToRawMessage(const std::wstring& xmlMessage, const std::vector<std::wstring>* urlList,
	std::wstring* rawMessage)
{
	HRESULT hr;
	IXmlReader *pReader = NULL;
	XmlNodeType nodeType;
	const WCHAR* pwszPrefix;
	const WCHAR* pwszLocalName;
	const WCHAR* pwszValue;
	UINT cwchPrefix;

	auto fullStr = L"<msg>" + xmlMessage + L"</msg>";
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
		}
		break;
		case XmlNodeType_Text:
		case XmlNodeType_Whitespace:
			if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
			{
				TRACE_IF(LOG_XML, L"Error getting value, error is %08.8lx", hr);
				HR(hr);
			}
			*rawMessage += pwszValue;
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
