// chatclient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ChatClient.h"
#include "../common/LoginCommand.h"
#include "../common/SockStream.h"

ChatClient::ChatClient(const std::wstring& ipaddr, int port)
{
	InetPton(AF_INET, ipaddr.c_str(), &serverAddr);
}

ChatClient::~ChatClient()
{
}

bool ChatClient::connect(const std::wstring& username, const std::wstring& password)
{
	sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addr = { 0 };
	addr.sin_port = htons(2333);
	addr.sin_family = AF_INET;
	IN_ADDR in_addr;
	inet_pton(AF_INET, "127.0.0.1", &in_addr);
	addr.sin_addr = in_addr;

	int ret = ::connect(sock_, (const sockaddr*)&addr, sizeof(sockaddr_in));
	assert(ret == 0);

	LoginCommand cmd;
	cmd.set(username, password);
	SockStream stream;
	cmd.writeTo(&stream);
	ret = ::send(sock_, stream.getBuff(), stream.getSize(), 0);
	assert(ret != SOCKET_ERROR);

	char keke[1024];
	ret = ::recv(sock_, keke, 1024, 0);
	if (ret > 0 && keke[0] == 1)
		return true;
	return false;
}
 void ChatClient::sendMessage(const std::wstring& username, const std::wstring& message) {
}