#include "stdafx.h"
#include "Client.h"
#include "ClientManager.h"
#include "User.h"

ClientManager::ClientManager()
{
}


ClientManager::~ClientManager()
{
}

HERRCODE ClientManager::addClient(const std::u16string& email, Client* client)
{
	std::lock_guard<std::recursive_mutex> lock(fMutex);
	fClientMap[email] = client;
	return H_OK;
}

HERRCODE ClientManager::removeClient(const std::u16string& email)
{
	std::lock_guard<std::recursive_mutex> lock(fMutex);
	fClientMap.erase(email);
	return H_OK;
}

HERRCODE ClientManager::setClientAddr(const std::u16string& email, const std::u16string& authKey,
				const sockaddr_in& localAddr, const sockaddr_in& publicAddr)
{
	std::lock_guard<std::recursive_mutex> lock(fMutex);
	if(!fClientMap.count(email))
		return H_FAILED;
	auto& client = fClientMap[email];
	if (client->getAuthKey() != authKey) {
		return H_AUTH_FAILED;
	} 
	client->setAddr(localAddr, publicAddr);
	return H_OK;
}

HERRCODE ClientManager::getClientSocket(const std::u16string& email, SOCKET* socket)
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

HERRCODE ClientManager::getEmailBySocket(SOCKET sock, std::u16string* email)
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
