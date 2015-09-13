#include "stdafx.h"
#include "../common/ChatOverlappedData.h"
#include "../common/Utils.h"
#include "../common/NetConstants.h"
#include "../common/trace.h"
#include "TcpPeerManager.h"
#include "ChatClient.h"

static const std::wstring kPeerSyncString = L"AvChatPeerSync";
const static int kMaxAcceptRequest = 1000;
LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs = nullptr;
LPFN_ACCEPTEX lpfnAcceptEx = nullptr;
#define LOG_IOCP 0
namespace avc
{
	TcpPeerManager::TcpPeerManager(ChatClient* client)
	{
		client_ = client;
	}

	TcpPeerManager::~TcpPeerManager()
	{
	}

	HERRCODE TcpPeerManager::createPeerSocket(IN_ADDR localAddr, HANDLE hComp, int compKey)
	{
		sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock_ == INVALID_SOCKET)
			return H_NETWORK_ERROR;

		sockaddr_in addr = { 0 };
		addr.sin_port = htons(0);
		addr.sin_family = AF_INET;
		addr.sin_addr = localAddr;

		int rc = bind(sock_, (const sockaddr*)&addr, sizeof(sockaddr_in));
		if (rc != 0) {
			return H_NETWORK_ERROR;
		}

		rc = listen(sock_, SOMAXCONN);
		if (rc != 0) {
			return H_NETWORK_ERROR;
		}

		HANDLE hComp2 = CreateIoCompletionPort((HANDLE)sock_, hComp, compKey, 0);
		if (hComp2 != hComp)
			return H_FAILED;

		int nameLen = sizeof(sockaddr_in);
		if (getsockname(sock_, (sockaddr*)&addr, &nameLen))
			return H_NETWORK_ERROR;
		port_ = addr.sin_port;
		base::Utils::QueueRecvCmdRequest(sock_, hComp, compKey);

		GUID guidAcceptEx = WSAID_ACCEPTEX;
		GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
		DWORD bytes;
		rc = WSAIoctl(sock_, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx,
			sizeof(GUID), &lpfnAcceptEx, sizeof(void*), &bytes, NULL, NULL);
		if (!(rc == 0 && lpfnAcceptEx)) {
			return H_NETWORK_ERROR;
		}
		rc = WSAIoctl(sock_, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidGetAcceptExSockaddrs,
			sizeof(GUID), &lpfnGetAcceptExSockaddrs, sizeof(void*), &bytes, NULL, NULL);
		if (!(rc == 0 && lpfnGetAcceptExSockaddrs)) {
			return H_NETWORK_ERROR;
		}
		return H_OK;
	}

	HERRCODE TcpPeerManager::connectToPeer(const std::wstring& remoteName, IN_ADDR remoteAddr, short remotePort, int compKey)
	{
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET)
			return H_NETWORK_ERROR;
		sockaddr_in addr = { 0 };
		addr.sin_port = htons(remotePort);
		addr.sin_family = AF_INET;
		addr.sin_addr = remoteAddr;
		if (::connect(sock, (const sockaddr*)&addr, sizeof(sockaddr_in)) != 0) {
			return H_NETWORK_ERROR;
		}
		auto randomString = base::Utils::GenerateRandomString(kPeerSyncString.size());
		SockStream os;
		os.writeInt(net::kCommandType_PeerSync);
		os.writeInt(0);
		os.writeString(kPeerSyncString);
		os.writeString(client_->getEmail());
		os.writeString(randomString);
		os.flushSize();
		auto rc = ::send(sock, os.getBuf(), os.getSize(), 0);
		if (rc == SOCKET_ERROR) {
			return H_NETWORK_ERROR;
		}
		buffer buf(200);
		rc = ::recv(sock, buf.data(), buf.size(), 0);
		if (rc) {
			SockStream is(buf.data(), rc);
			try {
				auto type = is.getInt();
				if (type != net::kCommandType_PeerSyncAck)
					return H_NETWORK_ERROR;
				auto size = is.getInt();
				if (rc != size)
					return H_NETWORK_ERROR;
				auto magicStr = is.getString();
				if (magicStr != kPeerSyncString)
					return H_NETWORK_ERROR;
				auto name = is.getString();
				if (name != remoteName)
					return H_NETWORK_ERROR;
				auto str = is.getString();
				if (str.size() != kPeerSyncString.size())
					return H_NETWORK_ERROR;
				str == base::Utils::XorString(str, kPeerSyncString);
				if (str != randomString)
					return H_NETWORK_ERROR;
				HANDLE hComp2 = CreateIoCompletionPort((HANDLE)sock, client_->getHandleComp(), compKey, 0);
				if (hComp2 != client_->getHandleComp())
					return H_FAILED;
				sockMap_[remoteName] = sock;
				return H_OK;
			} catch (...) {
				return H_NETWORK_ERROR;
			}
		} else {
			return H_NETWORK_ERROR;
		}
	}

	HERRCODE TcpPeerManager::tryToAcceptPeer()
	{
		if (acceptRequest_ < kMaxAcceptRequest) {
			ChatOverlappedData* overlap = new ChatOverlappedData(net::kAction_Accept);
			SOCKET acceptSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			overlap->setSocket(acceptSock);
			buffer& recvBuf = overlap->getBuf();
			BOOL rc = lpfnAcceptEx(sock_, acceptSock, recvBuf.data(), recvBuf.size() - (sizeof(sockaddr_in) + 16) * 2,
				sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, overlap);
			DWORD errCode = GetLastError();
			if (rc) {
				TRACE_IF(LOG_IOCP, "get an accepted sync\n");
			} else if (errCode == ERROR_IO_PENDING) {
				acceptRequest_++;
				TRACE_IF(LOG_IOCP, "%d queued an accept request\n", acceptRequest_);
			} else {
				return H_FAILED;
			}
		}
		return H_OK;
	}

	HERRCODE TcpPeerManager::acceptPeer(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key)
	{
		sockaddr* localAddr;
		sockaddr* remoteAddr;
		INT localAddrLen, remoteAddrLen;
		buffer& recvBuf = ol->getBuf();
		lpfnGetAcceptExSockaddrs(recvBuf.data(), recvBuf.size() - (sizeof(sockaddr_in) + 16) * 2,
			sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
			&localAddr, &localAddrLen, &remoteAddr, &remoteAddrLen);

		HANDLE hComp2 = CreateIoCompletionPort((HANDLE)ol->getSocket(), client_->getHandleComp(), ChatClient::kPeerListenKey, 0);
		if (hComp2 == client_->getHandleComp())
			return H_NETWORK_ERROR;
		acceptRequest_--;
		acceptedRequest_++;
		client_->onRecv(ol, bytes, key);
		return H_OK;
	}

	HERRCODE TcpPeerManager::onCmdPeerSync(SOCKET socket, SockStream& is)
	{
		int size;
		std::wstring remoteName;
		std::wstring remoteStr;
		try {
			size = is.getInt();
			if (size != is.getSize())
				return H_INVALID_PACKAGE;
			remoteName = is.getString();
			remoteStr = is.getString();
		} catch (...) {
			return H_INVALID_PACKAGE;
		}
		auto ackStr = base::Utils::XorString(remoteStr, kPeerSyncString);
		sockMap_[remoteName] = socket;
		SockStream  os;
		os.writeInt(net::kCommandType_PeerSyncAck);
		os.writeInt(0);
		os.writeString(kPeerSyncString);
		os.writeString(client_->getEmail());
		os.writeString(ackStr);
		os.flushSize();
		base::Utils::QueueSendRequest(socket, os, client_->getHandleComp(), ChatClient::kPeerListenKey);
		return H_OK;
	}

	HERRCODE TcpPeerManager::onCmdBuildP2pPath(SOCKET socket, SockStream& is)
	{
		HERRCODE hr;
		int size;
		int64_t id;
		std::wstring sender;
		int senderLocalIp;
		int senderPublicIp;
		short senderTcpPort;
		int recverPublicIp;
		try {
			size = is.getInt();
			if (size != is.getSize())
				return H_INVALID_PACKAGE;
			id = is.getInt64();
			sender = is.getString();
			senderLocalIp = is.getInt();
			senderPublicIp = is.getInt();
			senderTcpPort = is.getShort();
			recverPublicIp = is.getInt();
		} catch (...) {
			return H_INVALID_PACKAGE;
		}

		bool senderPublic = (senderLocalIp == senderPublicIp);
		sockaddr_in localAddr;
		int nameLen = sizeof(sockaddr_in);
		if (getsockname(socket, (sockaddr*)&localAddr, &nameLen)) {
			return H_NETWORK_ERROR;
		}
		bool recverPublic = (localAddr.sin_addr.S_un.S_addr == recverPublicIp);

		if (senderPublicIp) {
			IN_ADDR addr;
			addr.S_un.S_addr = senderPublicIp;
			hr = client_->peerMan().connectToPeer(sender, addr, senderTcpPort, ChatClient::kPeerConnKey);
			if (hr == H_OK) {
				SockStream os;
				os.writeInt(net::kCommandType_BuildPathAck);
				os.writeInt(0);
				os.writeInt64(id);
				os.writeString(client_->getEmail());
				os.writeString(sender);
				os.writeInt(net::kP2pAck_ConnTcp);
				os.flushSize();
				base::Utils::QueueSendRequest(socket, os, client_->getHandleComp(), ChatClient::kPeerConnKey);
				return H_OK;
			}
		}
		if (recverPublicIp) {
			SockStream os;
			os.writeInt(net::kCommandType_BuildPathAck);
			os.writeInt(0);
			os.writeInt64(id);
			os.writeString(client_->getEmail());
			os.writeString(sender);
			os.writeInt(net::kP2pAck_ListenTcp);
			os.writeInt(recverPublicIp);
			os.writeShort(port_);
			os.flushSize();
			base::Utils::QueueSendRequest(socket, os, client_->getHandleComp(), ChatClient::kServerKey);
			return H_OK;
		}
		//NAT
		return H_OK;
	}

	HERRCODE TcpPeerManager::onCmdBuildP2pPathAck(SOCKET socket, SockStream& is)
	{
		int size;
		int64_t id;
		std::wstring sender;
		std::wstring recver;
		int type;
		try {
			size = is.getInt();
			if (size != is.getSize())
				return H_INVALID_PACKAGE;
			id = is.getInt64();
			sender = is.getString();
			recver = is.getString();
			if (recver != client_->getEmail())
				return H_INVALID_PACKAGE;
			type = is.getInt();
		} catch (...) {
			return H_INVALID_PACKAGE;
		}

		if (type == net::kP2pAck_ListenTcp) { // sender listen tcp
			auto remoteIp = is.getInt();
			auto remotePort = is.getShort();
			sockaddr_in remoteAddr;
			memset(&remoteAddr, 0, sizeof(sockaddr_in));
			remoteAddr.sin_addr.S_un.S_addr = remoteIp;
			remoteAddr.sin_family = AF_INET;
			remoteAddr.sin_port = remotePort;
			auto hr = connectToPeer(sender, remoteAddr.sin_addr, remoteAddr.sin_port, ChatClient::kPeerConnKey);
			return hr;
		} else if (type == net::kP2pAck_TcpHole) {
			return H_OK;
		} else if (type == net::kP2pAck_UdpHole) {
			return H_OK;
		}
		return H_INVALID_PACKAGE;
	}

	HERRCODE TcpPeerManager::requestP2PConnect(SOCKET socket, const std::wstring& remote)
	{
		auto id = time(NULL);
		SockStream os;
		os.writeInt(net::kCommandType_BuildPath);
		os.writeInt(0);
		os.writeInt64(id);
		os.writeString(remote);
		sockaddr_in localAddr;
		int nameLen = sizeof(sockaddr_in);
		if (getsockname(socket, (sockaddr*)&localAddr, &nameLen)) {
			return H_NETWORK_ERROR;
		}
		os.writeInt(localAddr.sin_addr.S_un.S_addr);
		os.writeShort(port_);
		os.flushSize();
		base::Utils::QueueSendRequest(socket, os, client_->getHandleComp(), ChatClient::kServerKey);
		return H_OK;
	}
}
