#pragma once

class ChatOverlappedData;
class ChatCommand;
class MessageCommand;
class LoginCommand;

struct ClientState
{
	ChatOverlappedData* recvOl;
	ChatOverlappedData* sendOl;
	std::wstring username;
};

class ChatServer
{
public:
	ChatServer();
	~ChatServer();
	void init();
	void run();
	bool quit();

private:
	bool queueCompletionStatus();
	void onAccept(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key);
	void onRecv(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key);
	void dispatchCommand(ChatOverlappedData* ol, ChatCommand* cmd);
	void parseCommand(ChatOverlappedData* ol, char* recvBuf, int& bytes,
		int& neededSize, std::vector<ChatCommand*>& cmdVec);
	ChatCommand* getCommand(char* recvBuf, int bytes, buffer& cmdBuf);

	void onCmdLogin(LoginCommand* loginCmd, ChatOverlappedData* ol);
	void onCmdMessage(MessageCommand* messageCmd, ChatOverlappedData* ol);

	void updateUserlist();
	void addSocketMap(const std::wstring& username, const ClientState& cs);
	bool getClientByUsername(const std::wstring& username, ClientState* cs);
	void removeClientByUsername(const std::wstring& username);
	void send(ClientState* ol, char* buff, int len);
	void queueRecvCmdRequest(ChatOverlappedData* ol);

	std::map<std::wstring, ClientState> connMap_;
	std::mutex mapMutex_;
	SOCKET listenSock_;
	HANDLE hComp_;
	bool chatDataToClientState(ChatOverlappedData* ol, ClientState* cs);

	std::atomic<bool> quit_;
	std::atomic<int>  acceptRequest_;
	std::atomic<int> acceptedRequest_;
	static LPFN_ACCEPTEX        lpfnAcceptEx;
	static LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;
};