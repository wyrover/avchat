#pragma once

#include <stdint.h>
#include <string>
#include <atomic>
#include <vector>
#include <thread>
#include <Windows.h>
#include "../common/errcode.h"
#include "CommandCenter.h"
#include "ErrorManager.h"
#include "IChatClientController.h"
#include "RequestFilesInfo.h"
#include "TcpPeerManager.h"

class ChatOverlappedData;

namespace avc
{
	class ChatClient
	{
	public:
		const static int kServerKey = 233;
		const static int kPeerListenKey = 234;
		const static int kPeerConnKey = 235;

	public:
		ChatClient();
		~ChatClient();
		HERRCODE init(const std::wstring& serverAddr, int port);
		HERRCODE login(const std::wstring& username, const std::wstring& password);
		HERRCODE autoLogin(const std::wstring& username, const std::wstring& token);
		HERRCODE sendMessage(const std::wstring& username, const std::wstring& message, time_t timestamp);
		HERRCODE sendFileTransferRequest(const std::wstring& username, const RequestFilesInfo& fileInfo, time_t timestamp);
		HERRCODE confirmFileTransferRequest(const std::wstring& username, time_t timestamp, bool recv, const std::wstring& savePath);
		HERRCODE logout();

		ErrorManager& messageMan();
		TcpPeerManager& peerMan();
		IChatClientController* controller();

		void onRecv(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key);
		HANDLE getHandleComp();
		void queueCheckTimeoutTask();
		void setImageCacheDir(const std::wstring& filePath);
		std::wstring getImageDir();
		std::vector<std::wstring> getUserList();
		void setController(IChatClientController* controller);
		std::wstring getEmail();
		void start();

	private:
		HERRCODE loginImpl(int type, const std::wstring& username, const std::wstring& credential);
		bool queueCompletionStatus();
		void queueSendRequest(SOCKET socket, SockStream& stream);
		void threadFun(bool initRecv);
		void quit(bool wait);
		HERRCODE initSocks(SOCKET sock);

	private:
		HANDLE hComp_;
		std::vector<std::thread> threads_;
		std::atomic<bool> quit_;
		CommandCenter cmdCenter_;
		TcpPeerManager peerMan_;

		std::wstring email_;
		std::wstring authKey_;
		std::wstring imageCache_;

		SOCKET sock_;
		int serverPort_;
		std::wstring serverAddr_;


		IChatClientController* controller_;
		ErrorManager msgMan_;
	};
}
