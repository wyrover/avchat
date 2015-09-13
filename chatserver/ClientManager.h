#pragma once

#include "../common/errcode.h"
#include <string>
class Client;
class ClientManager
{
public:
	ClientManager();
	~ClientManager();
	HERRCODE addClient(const std::wstring& email, Client* client);
	HERRCODE removeClient(const std::wstring& email);
	HERRCODE getClientSocket(const std::wstring& email, SOCKET* socket);
	HERRCODE getEmailBySocket(SOCKET sock, std::wstring* email);

	std::map<std::wstring, Client*> fClientMap;
	std::recursive_mutex fMutex;
};