ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -Wfatal-errors -std=c++11
lib_LIBRARIES = libcommon.a
libcommon_a_SOURCES = \
				common/stdafx.h \
				common/trace.h \
				common/ChatCommand.h \
				common/NetConstants.h \
				common/SockStream.h \
				common/Utils.h \
				common/FileUtils.h \
				common/StringUtils.h \
				common/buffer.h \
				common/errcode.h \
				common/linux_trace.cpp \
				common/ChatCommand.cpp \
				common/NetConstants.cpp \
				common/SockStream.cpp \
				common/Utils.cpp \
				common/FileUtils.cpp \
				common/StringUtils.cpp

bin_PROGRAMS = server
server_SOURCES = \
				chatserver/stdafx.h \
				chatserver/CommandCenter.h \
				chatserver/unixserver.h \
				chatserver/Client.h \
				chatserver/DBContext.h \
				chatserver/ServerContext.h \
				chatserver/User.h \
				chatserver/ClientManager.h \
				chatserver/FileMan.h \
				chatserver/Utils.h \
				chatserver/main.cpp \
				chatserver/CommandCenter.cpp \
				chatserver/unixserver.cpp \
				chatserver/stdafx.cpp \
				chatserver/Client.cpp \
				chatserver/DBContext.cpp \
				chatserver/ServerContext.cpp \
				chatserver/User.cpp \
				chatserver/ClientManager.cpp \
				chatserver/FileMan.cpp \
				chatserver/Utils.cpp
server_LDADD = -lmysqlcppconn-static -lmysqlclient -lcryptopp libcommon.a 
server_LDFLAGS = -z defs
