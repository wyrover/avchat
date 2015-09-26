#pragma once

#include "../common/errcode.h"
#include <string>
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

		std::map<std::u16string, Client*> fClientMap;
		std::recursive_mutex fMutex;
};
