#pragma once

#include <stdint.h>
#include <string>
#include <atomic>
#include <vector>
#include <thread>
#include <Windows.h>
#include "../common/errcode.h"
#include "CommandCenter.h"
class ChatOverlappedData;

class IChatClientController {
public:
	virtual void onNewMessage(const std::wstring& sender, const std::wstring& recver,
		int64_t timestamp, const std::wstring& message) = 0;
	virtual void onNewUserList() = 0;
	virtual void onFileRequest(const std::wstring& sender, int64_t timestamp, bool isFolder,
		const std::wstring& filename, int64_t fileSize) = 0;
};

class ChatClient
{
public:
	ChatClient();
	~ChatClient();
	HERRCODE init(const std::wstring& serverAddr, int port);
	HERRCODE login(const std::wstring& username, const std::wstring& password);
	HERRCODE sendMessage(const std::wstring& username, const std::wstring& message, time_t timestamp);
	std::vector<std::wstring> getUserList();
	void setController(IChatClientController* controller);
	std::wstring getUsername();
	void quit(bool wait);
	void start();

private:
	HERRCODE initWinsock();
	bool queueCompletionStatus();
	void queueSendRequest(SOCKET socket, SockStream& stream);
	void onRecv(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key);
	void threadFun(bool initRecv);

private:
	HANDLE hComp_;
	std::vector<std::thread> threads_;
	std::atomic<bool> quit_;
	CommandCenter cmdCenter_;
	std::wstring username_;
	SOCKET sock_;
	sockaddr_in serverAddr_;
	IChatClientController* controller_;
};