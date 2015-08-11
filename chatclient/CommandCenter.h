#pragma once

#include "../common/buffer.h"
#include <mutex>
#include <vector>
#include <map>
class ChatCommand;
class SockStream;
class IChatClientController;
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
	void setController(IChatClientController* controller);
	std::vector<std::wstring> getUserList();

private:
	int handleCommand(SOCKET socket, buffer& cmdBuf, char* inBuf, int inLen);
	void onCmdMessage(SOCKET socket, const std::wstring& sender, const std::wstring& recver, time_t timestamp, const std::wstring& message);
	void onCmdUserList(SOCKET socket, const std::vector<std::wstring>& userList );

	void queueSendRequest(SOCKET socket, SockStream& stream);
	void queueRecvCmdRequest(SOCKET socket);
private:
	IChatClientController* controller_;
	std::map<SOCKET, CommandInfo> cmdMap_;
	std::vector<std::wstring> userList_;
	std::recursive_mutex userMutex_;
};