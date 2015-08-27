#pragma once

#include <stdint.h>
#include <string>
#include <atomic>
#include <vector>
#include <thread>
#include <Windows.h>
#include "../common/errcode.h"
#include "CommandCenter.h"
#include "MessageManager.h"
#include "IChatClientController.h"
class ChatOverlappedData;

class ChatClient
{
public:
	ChatClient();
	~ChatClient();
	HERRCODE init(const std::wstring& serverAddr, int port);
	HERRCODE login(const std::wstring& username, const std::wstring& password);
	HERRCODE autoLogin(const std::wstring& username, const std::wstring& token);
	HERRCODE sendMessage(const std::wstring& username, const std::wstring& message, time_t timestamp);
	HERRCODE logout();

	MessageManager& messageMan();
	IChatClientController* controller();

	void queueCheckTimeoutTask();
	void setImageCacheDir(const std::wstring& filePath);
	std::wstring getImageDir();
	std::vector<std::wstring> getUserList();
	void setController(IChatClientController* controller);
	std::wstring getEmail();
	void start();

private:
	HERRCODE initWinsock();
	HERRCODE loginImpl(int type, const std::wstring& username, const std::wstring& credential);
	bool queueCompletionStatus();
	void queueSendRequest(SOCKET socket, SockStream& stream);
	void onRecv(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key);
	void threadFun(bool initRecv);
	void quit(bool wait);

private:
	HANDLE hComp_;
	std::vector<std::thread> threads_;
	std::atomic<bool> quit_;
	CommandCenter cmdCenter_;
	std::wstring email_;
	std::wstring authKey_;
	std::wstring imageCache_;
	SOCKET sock_;
	sockaddr_in serverAddr_;
	IChatClientController* controller_;
	MessageManager msgMan_;
};