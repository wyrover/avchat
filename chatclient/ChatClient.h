#pragma once

class ChatClient
{
public:
	ChatClient(const std::wstring& ipaddr, int port);
	~ChatClient();
	bool connect(const std::wstring& username, const std::wstring& password);
	void sendMessage(const std::wstring& username, const std::wstring& message);
	
private:
	SOCKET sock_;
	sockaddr_in serverAddr;
};