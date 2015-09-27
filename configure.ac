#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([avchat], [1.0], [https://github.com/jichao/avchat])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_MACRO_DIR([m4])

AC_LANG([C++])
AM_INIT_AUTOMAKE([1.11 subdir-objects -Wall])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR

# Checks for libraries.
AC_SEARCH_LIBS([_ZTIN8CryptoPP14CBC_EncryptionE], [crypto++ cryptopp], 
		[],[AC_MSG_ERROR([avchat requires crypto++/cryptopp])])
AC_SEARCH_LIBS([mysql_init], [mysqlclient],
		[],[AC_MSG_ERROR([avchat requires mysql client library])])

# Checks for header files.
AC_CHECK_HEADERS([stdint.h string.h wchar.h])
AC_CHECK_HEADERS([iostream])
AC_CHECK_HEADERS([mysql_connection.h],
		[],[AC_MSG_ERROR([avchat requires mysql connector cpp])])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_C_RESTRICT
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([memchr memset socket])

AC_OUTPUT
