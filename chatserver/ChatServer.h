#pragma once
class ChatOverlappedData;
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
	void addSocketMap(const std::wstring& username, ChatOverlappedData* ol);
	SOCKET getSocketByUsername(const std::wstring& username);
	void removeSocketByUsername(const std::wstring& username);
	void send(SOCKET socket, char* buff, int len, ChatOverlappedData* ol);

	std::map<std::wstring, ChatOverlappedData*> connMap_;
	std::mutex mapMutex_;
	SOCKET listenSock_;
	HANDLE hComp_;
	std::atomic<bool> quit_;
	std::atomic<int>  acceptRequest_;
	std::atomic<int> acceptedRequest_;
	static LPFN_ACCEPTEX        lpfnAcceptEx;
	static LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;
};