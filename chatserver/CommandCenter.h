#pragma once

#include "ClientManager.h"
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
	void onCmdLogin(SOCKET socket, const std::wstring& email, const std::wstring& password);
	void onCmdMessage(SOCKET socket, const std::wstring& sender, const std::wstring& recver, const std::wstring& message);

	void updateUserlist();
	void queueSendRequest(SOCKET socket, SockStream& stream);
	void queueRecvCmdRequest(SOCKET socket);

private:
	std::map<SOCKET, CommandInfo> cmdMap_;
	ClientManager clientMan_;
};