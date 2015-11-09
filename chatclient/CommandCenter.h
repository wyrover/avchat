#pragma once

#include "../common/buffer.h"
#include <mutex>
#include <vector>
#include <map>
#include <thread>
#include <queue>

class ChatCommand;
class SockStream;

namespace avc
{
class ChatClient;
class ImageMessageForSend;
class IChatClientController;

struct CommandInfo
{
	CommandInfo() {
		fCmdLen = 0;
		fNeededLen = -1;
	}
	buffer fBuf;
	int fNeededLen;
	int fCmdLen;
	std::recursive_mutex fMutex;
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
	CommandCenter();
	~CommandCenter();
	int fill(SOCKET socket, char* data, int len);
	void setController(ChatClient* client, IChatClientController* controller);
	std::vector<std::u16string> getUserList();
	void postCommand(SOCKET socket, buffer& cmdBuf);
	void start();
	void quit();

private:
	void threadFun();
	int handleCommand(SOCKET socket, buffer& cmdBuf);
	void onCmdFileDownloadAck(SOCKET socket, SockStream& is);
	void onCmdFileTransferRequest(SOCKET socket, SockStream& is);

	HERRCODE onCmdFileCheckAck(SOCKET socket, SockStream& is);
	HERRCODE onCmdFileUploadAck(SOCKET socket, SockStream& is);
	HERRCODE onCmdImageMessage(SOCKET socket, SockStream& is);
	HERRCODE onCmdLoginAck(SOCKET socket, SockStream& is);
	HERRCODE onCmdUserList(SOCKET socket, SockStream& is);
	HERRCODE onCmdMessageAck(SOCKET socket, SockStream& is);
	HERRCODE onCmdMessage(SOCKET socket, SockStream& is);
	HERRCODE onCmdFileTransferRequestAck(SOCKET socket, SockStream& is);

	void queueSendRequest(SOCKET socket, SockStream& is);
	void queueRecvCmdRequest(SOCKET socket);

private:
	IChatClientController* controller_;
	ChatClient* client_;
	std::deque<CommandRequest> cmdQueue_;
	std::vector<std::thread> threads_;
	std::map<SOCKET, CommandInfo> cmdMap_;
	std::vector<std::u16string> userList_;
	std::recursive_mutex userMutex_;
	std::mutex mutex_;
	std::condition_variable cv_;
	bool quit_;
};
}
