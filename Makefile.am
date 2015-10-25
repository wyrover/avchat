ACLOCAL_AMFLAGS = -I m4
AM_CXXFLAGS = -Wfatal-errors -std=c++11
lib_LIBRARIES = libcommon.a libchatclient.a
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
	common/linux_trace.h \
	common/linux_trace.cpp \
	common/ChatCommand.cpp \
	common/NetConstants.cpp \
	common/SockStream.cpp \
	common/Utils.cpp \
	common/FileUtils.cpp \
	common/StringUtils.cpp

libchatclient_a_SOURCES = \
	chatclient/ChatClient.cpp \
	chatclient/ErrorManager.h \
	chatclient/RequestFilesInfo.cpp \
	chatclient/ChatClient.h \
	chatclient/FileManager.cpp \
	chatclient/RequestFilesInfo.h \
	chatclient/chatclient.vcxproj \
	chatclient/FileManager.h \
	chatclient/stdafx.cpp \
	chatclient/chatclient.vcxproj.filters \
	chatclient/IChatClientController.h \
	chatclient/stdafx.h \
	chatclient/ChatError.cpp \
	chatclient/ImageMessageForRecv.cpp \
	chatclient/targetver.h \
	chatclient/ChatError.h \
	chatclient/ImageMessageForRecv.h \
	chatclient/TcpPeerManager.cpp \
	chatclient/ChatRequest.cpp \
	chatclient/ImageMessageForSend.cpp \
	chatclient/TcpPeerManager.h \
	chatclient/ChatRequest.h \
	chatclient/ImageMessageForSend.h \
	chatclient/Utils.cpp \
	chatclient/MessageError.cpp \
	chatclient/Utils.h \
	chatclient/CommandCenter.cpp \
	chatclient/MessageError.h \
	chatclient/UtilsTest.cpp \
	chatclient/CommandCenter.h \
	chatclient/MessageRequest.cpp \
	chatclient/UtilsTest.h \
	chatclient/common_inc.h \
	chatclient/MessageRequest.h \
	chatclient/ErrorManager.cpp \
	chatclient/LoginError.h \
	chatclient/LoginError.cpp

bin_PROGRAMS = server chattest
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

test_SOURCES = \
	chattest/main.cpp \
	chattest/ChatClientController.h \
	chattest/ChatClientController.cpp

server_CXXFLAGS = $(AM_CXXFLAGS) $(MYSQL_CFLAGS) $(CRYPTOPP_CFLAGS)
server_LDADD = $(MYSQL_LDFLAGS) -lmysqlcppconn-static -lmysqlclient -lcryptopp -lconfig libcommon.a
server_LDFLAGS = -pthread

test_LDFLAGS = -pthread
test_LDADD = libchatclient.a libcommon.a $(CRYPTOPP_LDFLAGS) -lcryptopp 
test_CXXFLAGS = $(AM_CXXFLAGS)  $(CRYPTOPP_CFLAGS)
test_DEPENDENCIES = libchatclient.a libcommon.a

