// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <thread>
#include <memory>
#include <map>
#include <mutex>
#include <atomic>
#include <vector>
#include <string>
#include <algorithm>
#include <codecvt>
#include <locale>

// TODO: reference additional headers your program requires here
#include <stdio.h>
#include <assert.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <Ws2spi.h>
#include <ws2tcpip.h>
#include <MSWSock.h>
#include <Shlobj.h>
#include <Shlwapi.h>	

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <mysql_error.h>
#include <cppconn/prepared_statement.h>
#include <boost/algorithm/string.hpp>

#include "../common/buffer.h"
