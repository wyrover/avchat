#pragma once

#include "ClientManager.h"
#include "FileMan.h"
#include <mutex>
#include <condition_variable>
#include <deque>

class ChatServer;
class ChatCommand;
class SockStream;

struct CommandInfo
{
	CommandInfo() {
		fNeededLen = -1;
	}
	buffer fBuf;
	int fNeededLen;
	int fCmdLen;
};

struct CommandRequest
{
	buffer fBuf;
	SOCKET fSock;
	CommandRequest(SOCKET socket, buffer& buf) {
		fSock = socket;
		fBuf.swap(buf);
	}
};

class CommandCenter
{
public:
	CommandCenter(ChatServer* server);
	~CommandCenter();
	int fill(SOCKET socket, char* data, int len);
	void start();
	void quit();
	ClientManager& clientMan();

private:
	void onCmdLogin(SOCKET socket, SockStream& stream);
	void onCmdLogout(SOCKET socket, SockStream& stream);
	HERRCODE onCmdMessage(SOCKET socket, SockStream& stream);
	void onCmdFileCheck(SOCKET socket, int id, const std::vector<std::u16string>& hashList);
	void onCmdFileUpload(SOCKET socket, SockStream& stream);
	void onCmdFileDownload(SOCKET socket, SockStream& stream);
	void onCmdFileTransferRequest(SOCKET socket, SockStream& stream);
	void onCmdFileTransferRequestAck(SOCKET socket, SockStream& stream);
	HERRCODE onCmdBuildP2pPath(SOCKET socket, SockStream& stream);
	HERRCODE onCmdBuildP2pPathAck(SOCKET socket, SockStream& stream);

	void updateUserlist();
	void queueSendRequest(SOCKET socket, SockStream& stream);
	void queueRecvCmdRequest(SOCKET socket);
	void threadFun();

	void postCommand(SOCKET socket, buffer& cmdBuf);
	int handleCommand(SOCKET socket, buffer& cmdBuf);

private:
	std::deque<CommandRequest> cmdQueue_;
	std::vector<std::thread> threads_;
	std::map<SOCKET, CommandInfo> cmdMap_;
	volatile bool quit_;
	ClientManager clientMan_;
	FileMan fileMan_;
	ChatServer* server_;
	std::mutex mutex_;
	std::condition_variable cv_;
};

