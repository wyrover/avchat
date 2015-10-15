#pragma once
#ifndef _CRT_WIDE
#define __CRT_WIDE(_String) L ## _String
#define _CRT_WIDE(_String) __CRT_WIDE(_String)
#endif

#ifndef _CRT_APPEND
#define __CRT_APPEND(_Value1, _Value2) _Value1 ## _Value2
#define _CRT_APPEND(_Value1, _Value2) __CRT_APPEND(_Value1, _Value2)
#endif

#define __WSHORT_FILE__ (wcsrchr(_CRT_WIDE(__FILE__), '\\') ? wcsrchr(_CRT_WIDE(__FILE__), '\\') + 1 : _CRT_WIDE(__FILE__))


