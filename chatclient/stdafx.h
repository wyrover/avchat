// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <stdio.h>
#include <tchar.h>
#include <thread>
#include <memory>
#include <map>
#include <mutex>
#include <atomic>
#include <vector>
#include <string>
#include <condition_variable>
#include <stdio.h>      // C file management
#include <exception>    // std::exception
#include <ios>          // std::hex
#include <iostream>     // console output
#include <sstream>      // std::ostringstream
#include <stdexcept>    // std::runtime_error
#include <string>       // std::wstring
#include <vector>       // std::vector

// TODO: reference additional headers your program requires here
#include <stdio.h>
#include <assert.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <Ws2spi.h>
#include <ws2tcpip.h>
#include <MSWSock.h>
#include <Shlwapi.h>
#include <bcrypt.h>     // Cryptography API
#include <xmllite.h>
#include <ole2.h>
#include <strsafe.h>
#include <process.h>

#pragma comment(lib, "bcrypt.lib")

#include "../common/buffer.h"
