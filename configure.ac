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
AC_SEARCH_LIBS([config_init], [config], 
		[],[AC_MSG_ERROR([avchat requires libconfig])])


MYSQL_CONFIG=$(which mysql_config)
if test "$MYSQL_CONFIG"; then
	MYSQL_LDFLAGS=$(${MYSQL_CONFIG} --libs_r)
	MYSQL_CFLAGS=$(${MYSQL_CONFIG} --include)
else
	for i in /usr/local/mysql /usr/local /usr; do
		if test -r $i/include/mysql/mysql.h; then
			MYSQ_LIB_DIR=$i/lib/mysql
			MYSQL_INC_DIR=$i/include/mysql
			break
		elif test -r $i/include/mysql.h; then
			MYSQL_LIB_DIR=$i/lib
			MYSQL_INC_DIR=$i/include
		fi
	done
	if test x${MYSQL_LIB_DIR} != x; then
		MYSQL_LDFLAGS="-L{MYSQL_LIB_DIR}"
		MYSQL_CFLAGS="-I{MYSQL_INC_DIR}"
		AC_SUBST([MYSQL_CFLAGS])
		AC_SUBST([MYSQL_LDFLAGS])
	else
		AC_MSG_ERROR([avchat requires mysql client])
	fi
fi

XML2_CONFIG=$(which xml2-config)
if test "$MYSQL_CONFIG"; then
	XML2_LDFLAGS=$(${XML2_CONFIG} --libs)
	XML2_CFLAGS=$(${XML2_CONFIG} --cflags)
else
	AC_MSG_ERROR([avchat requires libxml2])
fi
AC_SUBST([XML2_LDFLAGS])
AC_SUBST([XML2_CFLAGS])

PKG_CHECK_MODULES([CRYPTOPP], [libcryptopp >= 5])
AC_SUBST([CRYPTOPP_CFLAGS])


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
