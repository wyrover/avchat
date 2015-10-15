#pragma once

#include "../common/errcode.h"
#include <map>
#include <netinet/in.h>
#include <atomic>
namespace avc
{
	class ChatClient;

	class TcpPeerManager
	{
	public:
		TcpPeerManager(ChatClient* client);
		~TcpPeerManager();
		HERRCODE createPeerSocket(in_addr localAddr, HANDLE hComp, int compKey);
		HERRCODE connectToPeer(const std::u16string& remoteName, in_addr remoteAddr, short remotePort, int compKey);
		//HERRCODE acceptPeer(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key);
		HERRCODE tryToAcceptPeer();
		HERRCODE onCmdPeerSync(SOCKET socket, SockStream& is);
		HERRCODE onCmdBuildP2pPath(SOCKET socket, SockStream& is);
		HERRCODE onCmdBuildP2pPathAck(SOCKET socket, SockStream& is);
		HERRCODE requestP2PConnect(SOCKET socket, const std::u16string& remote);

	private:
		SOCKET sock_;
		short port_;
		ChatClient* client_;
		std::atomic<int>  acceptRequest_;
		std::atomic<int> acceptedRequest_;
		std::map<std::u16string, SOCKET> sockMap_;
	};
}
