#pragma once

#include "ClientManager.h"
#include "FileMan.h"
#include <mutex>
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
	std::recursive_mutex fMutex;
	int fCmdLen;
};

class CommandCenter
{
public:
	CommandCenter(ChatServer* server);
	~CommandCenter();
	int fill(SOCKET socket, char* data, int len);

private:
	int handleCommand(SOCKET socket, buffer& cmdBuf, char* inBuf, int inLen);
	void onCmdLogin(SOCKET socket, SockStream& stream);
	void onCmdLogout(SOCKET socket, SockStream& stream);
	HERRCODE onCmdMessage(SOCKET socket, SockStream& stream);
	void onCmdFileCheck(SOCKET socket, int id, const std::vector<std::u16string>& hashList);
	void onCmdFileUpload(SOCKET socket, SockStream& stream);
	void onCmdFileDownload(SOCKET socket, SockStream& stream);
	void onCmdFileTransferRequest(SOCKET socket, SockStream& stream);
	void onCmdFileTransferRequestAck(SOCKET socket, SockStream& stream);
	void onCmdBuildP2pPath(SOCKET socket, SockStream& stream);
	HERRCODE onCmdBuildP2pPathAck(SOCKET socket, SockStream& stream);

	void updateUserlist();
	void queueSendRequest(SOCKET socket, SockStream& stream);
	void queueRecvCmdRequest(SOCKET socket);

private:
	std::map<SOCKET, CommandInfo> cmdMap_;
	ClientManager clientMan_;
	FileMan fileMan_;
	ChatServer* server_;
	std::mutex mapMutex_;
};

