#pragma once

#include "stdafx.h"
#include "../common/errcode.h"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
class Client;
class ClientManager
{
	public:
		ClientManager();
		~ClientManager();
		HERRCODE addClient(const std::u16string& email, Client* client);
		HERRCODE removeClient(const std::u16string& email);
		HERRCODE getClientSocket(const std::u16string& email, SOCKET* socket);
		HERRCODE getEmailBySocket(SOCKET sock, std::u16string* email);
		HERRCODE setClientAddr(const std::u16string& email, const std::u16string& authKey,
				const sockaddr_in& localAddr, const sockaddr_in& publicAddr);

		std::map<std::u16string, Client*> fClientMap;
		std::recursive_mutex fMutex;
};
