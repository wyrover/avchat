#pragma once

#include <stdint.h>
#include <string>
#include <atomic>
#include <vector>
#include <thread>
#include <sys/event.h>
typedef int SOCKET;
typedef int HANDLE;
typedef int DWORD;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#include "../common/errcode.h"
#include "CommandCenter.h"
#include "ErrorManager.h"
#include "IChatClientController.h"
#include "RequestFilesInfo.h"
#include "TcpPeerManager.h"

namespace avc
{
	struct LoginRequest
	{
		std::u16string username;
		std::u16string password;
		int type;
	};

	struct LoginResult
	{
		bool succeeded;
		int type;
		std::u16string authKey;
	};

	class ChatClient
	{
		public:
			ChatClient();
			~ChatClient();
			HERRCODE init(const std::u16string& serverAddr, int port);
			HERRCODE login(const std::u16string& username, const std::u16string& password);
			HERRCODE autoLogin(const std::u16string& username, const std::u16string& token);
			HERRCODE sendMessage(const std::u16string& username, const std::u16string& message, time_t timestamp);
			HERRCODE sendFileTransferRequest(const std::u16string& username, const RequestFilesInfo& fileInfo, time_t timestamp);
			HERRCODE confirmFileTransferRequest(const std::u16string& username, time_t timestamp, bool recv, const std::u16string& savePath);
			HERRCODE logout();

			ErrorManager& messageMan();
			TcpPeerManager& peerMan();
			IChatClientController* controller();

			void queueCheckTimeoutTask();
			void setImageCacheDir(const std::u16string& filePath);
			std::u16string getImageDir();
			std::vector<std::u16string> getUserList();
			void setController(IChatClientController* controller);
			std::u16string getEmail();
			void start();

		private:
			HERRCODE loginImpl(int type, const std::u16string& username, const std::u16string& credential);
			HERRCODE handleConnect();
			void queueSendRequest(SOCKET socket, SockStream& stream);
			void threadFun(bool initRecv);
			void quit(bool wait);
			HERRCODE initSock();

		private:
			std::vector<std::thread> threads_;
			std::atomic<bool> quit_;
			CommandCenter cmdCenter_;
			TcpPeerManager peerMan_;

			std::u16string email_;
			std::u16string authKey_;
			std::u16string imageCache_;

			SOCKET sock_;
			SOCKET listenSock_;
			std::atomic<int> acceptedRequest_;

			int kq_;
			int serverPort_;
			std::u16string serverAddr_;
			LoginRequest loginRequest_;

			IChatClientController* controller_;
			ErrorManager msgMan_;
	};
}
