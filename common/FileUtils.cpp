#include "stdafx.h"
#include <strsafe.h>
#include "FileUtils.h"

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


}
FileUtils::FileUtils()
{
}


FileUtils::~FileUtils()
{
}

bool FileUtils::DirExists(LPCWSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool FileUtils::FileExists(LPCWSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool FileUtils::PathExists(LPCWSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES);
}

int64_t FileUtils::GetFolderSize(const std::wstring& dirPath)
{
	int64_t size = 0;
	if (!DirExists(dirPath.c_str())) {
		return -1;
	} else {
		WIN32_FIND_DATA findData;
		HANDLE hFind = FindFirstFile(dirPath.c_str(), &findData);
		if (hFind == INVALID_HANDLE_VALUE)
			return -1;
		do {
			if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				size += GetFolderSize(findData.cFileName);
			} else {
				HANDLE hFile = CreateFile(findData.cFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
				if (hFile != INVALID_HANDLE_VALUE) {
					LARGE_INTEGER li = { 0 };
					GetFileSizeEx(hFile, &li);
					size += li.QuadPart;
					CloseHandle(hFile);
				}
			}
		} while (FindNextFile(hFind, &findData) != 0);
		FindClose(hFind);
	}
	return size;
}

int64_t FileUtils::FnGetFileSize(const std::wstring& filePath)
{
	HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	LARGE_INTEGER li;
	bool result = !!GetFileSizeEx(hFile, &li);
	CloseHandle(hFile);
	if (!result) {
		return -1;
	} else {
		return li.QuadPart;
	}
}

bool FileUtils::ReadAll(const std::wstring& filePath, buffer& outBuf)
{
	buffer readBuf;
	HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	LARGE_INTEGER li;
	bool result = !!GetFileSizeEx(hFile, &li);
	if (li.QuadPart > INT_MAX)
		return false;
	readBuf.size((size_t)li.QuadPart);
	DWORD bytesRead;
	if (!ReadFile(hFile, readBuf.data(), readBuf.size(), &bytesRead, NULL) || bytesRead != readBuf.size()) {
		CloseHandle(hFile);
		return false;
	} else {
		CloseHandle(hFile);
		readBuf.swap(outBuf);
		return true;
	}
}

bool FileUtils::FnCreateFile(const std::wstring& filePath, buffer& buf)
{
	HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD bytesWritten;
	if (!WriteFile(hFile, buf.data(), buf.size(), &bytesWritten, NULL) || bytesWritten != buf.size()) {
		CloseHandle(hFile);
		return false;
	} else {
		CloseHandle(hFile);
		return true;
	}
}

int FileUtils::CalculateFileSHA1(const std::wstring& filePath, std::wstring* pHash)
{
	try {
		std::wstring hash = HashFileSHA1(filePath);
		*pHash = hash;
		return 0;
	} catch (const std::exception& e) {
		return -1;
	}
}

std::wstring FileUtils::getFileExt(const std::wstring& filePath)
{
	return (PathFindExtension((PTSTR)filePath.c_str()) + 1);
}

void FileUtils::MkDirs(const std::wstring& dirPath)
{
	SHCreateDirectoryEx(NULL, dirPath.c_str(), NULL);
}

std::wstring FileUtils::FileSizeToReadable(int fileSize)
{
	wchar_t buf[20];
	double size = fileSize;
	int i = 0;
	const wchar_t* units[] = { L"B", L"kB", L"MB", L"GB", L"TB", L"PB", L"EB", L"ZB", L"YB" };
	while (size > 1024) {
		size /= 1024;
		i++;
	}
	StringCchPrintf(buf, 20, L"%.*f %s", i, size, units[i]);
	return buf;
}
