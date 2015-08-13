// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <assert.h>
#include <thread>
#include <memory>
#include <map>
#include <mutex>
#include <atomic>
#include <vector>
#include <codecvt>
#include <locale>
#include <iostream>
#include <sstream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <tchar.h>
#include <Ws2spi.h>
#include <ws2tcpip.h>
#include <MSWSock.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <bcrypt.h>     // Cryptography API#include <windows.h>