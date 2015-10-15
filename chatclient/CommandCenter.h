#pragma once

#include "../common/buffer.h"
#include <mutex>
#include <vector>
#include <map>

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
			fNeededLen = -1;
		}
		buffer fBuf;
		int fNeededLen;
        int fCmdLen;
        std::recursive_mutex fMutex;
	};

	class CommandCenter
	{
	public:
		CommandCenter();
		~CommandCenter();
		int fill(SOCKET socket, char* data, int len);
		void setController(ChatClient* client, IChatClientController* controller);
		std::vector<std::u16string> getUserList();
		void sendImageMessage(SOCKET socket, ImageMessageForSend *message);

	private:
        int handleCommand(SOCKET socket, buffer& cmdBuf);
		void onCmdFileCheckAck(SOCKET socket, ImageMessageForSend* msg, const std::vector<std::u16string>& urlList);
		void onCmdFileUploadAck(SOCKET socket, ImageMessageForSend* msg, const std::vector<std::u16string>& urlList);
		void onCmdFileDownloadAck(SOCKET socket, SockStream& is);
		void onCmdFileTransferRequest(SOCKET socket, SockStream& is);

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
		std::map<SOCKET, CommandInfo> cmdMap_;
		std::vector<std::u16string> userList_;
        std::recursive_mutex userMutex_;
        std::mutex mapMutex_;
	};
}
