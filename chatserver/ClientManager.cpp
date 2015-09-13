#include "stdafx.h"
#include "client.h"
#include "ClientManager.h"
#include "User.h"


ClientManager::ClientManager()
{
}


ClientManager::~ClientManager()
{
}

HERRCODE ClientManager::addClient(const std::wstring& email, Client* client)
{
	std::lock_guard<std::recursive_mutex> lock(fMutex);
	fClientMap[email] = client;
	return H_OK;
}

HERRCODE ClientManager::removeClient(const std::wstring& email)
{
	std::lock_guard<std::recursive_mutex> lock(fMutex);
	auto client = fClientMap[email];
	fClientMap.erase(email);
	return H_OK;
}

HERRCODE ClientManager::getClientSocket(const std::wstring& email, SOCKET* socket)
{
	std::lock_guard<std::recursive_mutex> lock(fMutex);
	auto client = fClientMap[email];
	if (!client) {
		*socket = INVALID_SOCKET;
		return H_FAILED;
	}
	*socket = client->getSocket();
	return H_OK;
}

HERRCODE ClientManager::getEmailBySocket(SOCKET sock, std::wstring* email)
{
	std::lock_guard<std::recursive_mutex> lock(fMutex);
	for (auto& item : fClientMap) {
		if (item.second->getSocket() == sock) {
			*email = item.second->getEmail();
			return H_OK;
		}
	}
	return H_FAILED;
}
