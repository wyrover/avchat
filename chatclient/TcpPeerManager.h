#pragma once

#include "../common/errcode.h"
#include <map>
#include <atomic>
namespace avc
{
	class ChatClient;

	class TcpPeerManager
	{
	public:
		TcpPeerManager(ChatClient* client);
		~TcpPeerManager();
		HERRCODE createPeerSocket(IN_ADDR localAddr, HANDLE hComp, int compKey);
		HERRCODE connectToPeer(const std::wstring& remoteName, IN_ADDR remoteAddr, short remotePort, int compKey);
		HERRCODE acceptPeer(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key);
		HERRCODE tryToAcceptPeer();
		HERRCODE onCmdPeerSync(SOCKET socket, SockStream& is);
		HERRCODE onCmdBuildP2pPath(SOCKET socket, SockStream& is);
		HERRCODE onCmdBuildP2pPathAck(SOCKET socket, SockStream& is);
		HERRCODE requestP2PConnect(SOCKET socket, const std::wstring& remote);

	private:
		SOCKET sock_;
		short port_;
		ChatClient* client_;
		std::atomic<int>  acceptRequest_;
		std::atomic<int> acceptedRequest_;
		std::map<std::wstring, SOCKET> sockMap_;
	};
}