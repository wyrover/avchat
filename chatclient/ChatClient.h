#pragma once

class ChatOverlappedData;
class MessageCommand;
class ChatCommand;
class IChatClientController {
public:
	virtual void onNewMessage(const std::wstring& username, const std::wstring& message) = 0;
	virtual void onNewUserList() = 0;
};

class ChatClient
{
public:
	ChatClient(const std::wstring& serverAddr, int port);
	~ChatClient();
	bool loginIn(const std::wstring& username, const std::wstring& password);
	void sendMessage(const std::wstring& username, const std::wstring& message);
	std::vector<std::wstring> getUserList();
	void quit();
	void run();
	bool isValid();
	
private:
	bool queueCompletionStatus();
	void dispatchCommand(ChatOverlappedData* ol, ChatCommand* cmd);
	void parseCommand(ChatOverlappedData* ol, char* recvBuf, int& bytes,
		int& neededSize, std::vector<ChatCommand*>& cmdVec);
	ChatCommand* getCommand(char* recvBuf, int bytes, buffer& cmdBuf);
	void onRecv(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key);

	void onCmdLoginAck(int ret, ChatOverlappedData* ol);
	void onCmdMessage(MessageCommand* messageCmd, ChatOverlappedData* ol);
	void onCmdUserList(const std::vector<std::wstring>& userList, ChatOverlappedData* ol);
	void queueRecvCmdRequest(ChatOverlappedData* ol);
	void send(SOCKET socket, char* buff, int len, ChatOverlappedData* ol);
	void waitRun();

private:
	bool loggedIn_;
	SOCKET sock_;
	sockaddr_in serverAddr_;
	bool quit_;
	HANDLE hComp_;
	IChatClientController* controller_;
	std::vector<std::wstring> userList_;
	std::wstring userName_;
	std::mutex userMutex_;
	HANDLE hRun_;
	bool valid_;
	ChatOverlappedData* ol;
};