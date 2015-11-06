#pragma once

#include <atomic>
#include <map>
#include <vector>
#include <thread>
#include <netinet/in.h>
#include "../common/errcode.h"

namespace avc
{
	class ChatClient;

	class TcpPeerManager
	{
	public:
		TcpPeerManager(ChatClient* client);
		~TcpPeerManager();
		HERRCODE initSock();
		HERRCODE connectToPeer(const std::u16string& remoteName, const sockaddr_in& remoteAddr,
				const std::u16string& authKey);
		HERRCODE createPeerSocket(in_addr localAddr, HANDLE hComp, int compKey);
		//HERRCODE acceptPeer(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key);
		HERRCODE tryToAcceptPeer();
		HERRCODE onCmdPeerSync(SOCKET socket, SockStream& is);
		HERRCODE onCmdBuildP2pPath(SOCKET socket, SockStream& is);
		HERRCODE onCmdBuildP2pPathAck(SOCKET socket, SockStream& is);
		HERRCODE requestP2PConnect(SOCKET socket, const std::u16string& remote);

	private:
		void netWorkerFunc(); 	
		void timerCheckFunc();
		int acceptConnections(SOCKET sock);
		bool quit();

	private:
		SOCKET sock_;
		short port_;
		int kq_;
		ChatClient* client_;
		std::u16string authKey_;
		std::atomic<int>  acceptRequest_;
		std::atomic<int> acceptedRequest_;
		std::map<std::u16string, SOCKET> sockMap_;
		std::vector<std::thread> threads_;
		volatile bool quit_;
	};
}
