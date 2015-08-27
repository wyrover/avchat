#pragma once

#include "ClientManager.h"
#include "FileMan.h"

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
class CommandCenter
{
public:
	CommandCenter();
	~CommandCenter();
	int fill(SOCKET socket, char* data, int len);

private:
	int handleCommand(SOCKET socket, buffer& cmdBuf, char* inBuf, int inLen);
	void onCmdLogin(SOCKET socket, SockStream& stream);
	void onCmdLogout(SOCKET socket, SockStream& stream);
	void onCmdMessage(SOCKET socket, SockStream& stream);
	void onCmdFileCheck(SOCKET socket, int id, const std::vector<std::wstring>& hashList);
	void onCmdFileUpload(SOCKET socket, SockStream& stream);
	void onCmdFileDownload(SOCKET socket, SockStream& stream);

	void updateUserlist();
	void queueSendRequest(SOCKET socket, SockStream& stream);
	void queueRecvCmdRequest(SOCKET socket);

private:
	std::map<SOCKET, CommandInfo> cmdMap_;
	ClientManager clientMan_;
	FileMan fileMan_;
};