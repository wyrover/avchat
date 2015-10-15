#include "stdafx.h"
#include "../common/Utils.h"
#include "../common/NetConstants.h"
#include "../common/trace.h"
#include "TcpPeerManager.h"
#include "ChatClient.h"

static const std::u16string kPeerSyncString = u"AvChatPeerSync";
const static int kMaxAcceptRequest = 1000;
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

	HERRCODE TcpPeerManager::createPeerSocket(in_addr localAddr, HANDLE hComp, int compKey)
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

		return H_OK;
	}

	HERRCODE TcpPeerManager::connectToPeer(const std::u16string& remoteName, in_addr remoteAddr, short remotePort, int compKey)
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
		return H_OK;
	}


	HERRCODE TcpPeerManager::onCmdPeerSync(SOCKET socket, SockStream& is)
	{
		int size;
		std::u16string remoteName;
		std::u16string remoteStr;
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
		return H_OK;
	}

	HERRCODE TcpPeerManager::onCmdBuildP2pPath(SOCKET socket, SockStream& is)
	{
		HERRCODE hr;
   /*     int size;*/
		//int64_t id;
		//std::u16string sender;
		//int senderLocalIp;
		//int senderPublicIp;
		//short senderTcpPort;
		//int recverPublicIp;
		//try {
			//size = is.getInt();
			//if (size != is.getSize())
				//return H_INVALID_PACKAGE;
			//id = is.getInt64();
			//sender = is.getString();
			//senderLocalIp = is.getInt();
			//senderPublicIp = is.getInt();
			//senderTcpPort = is.getShort();
			//recverPublicIp = is.getInt();
		//} catch (...) {
			//return H_INVALID_PACKAGE;
		//}

		//bool senderPublic = (senderLocalIp == senderPublicIp);
		//sockaddr_in localAddr;
		//socklen_t nameLen = sizeof(sockaddr_in);
		//if (getsockname(socket, (sockaddr*)&localAddr, &nameLen)) {
			//return H_NETWORK_ERROR;
		//}
		//bool recverPublic = (localAddr.sin_addr.s_addr == recverPublicIp);

		//if (senderPublicIp) {
			//in_addr addr;
			//addr.s_addr = senderPublicIp;
			//hr = client_->peerMan().connectToPeer(sender, addr, senderTcpPort);
			//if (hr == H_OK) {
				//SockStream os;
				//os.writeInt(net::kCommandType_BuildPathAck);
				//os.writeInt(0);
				//os.writeInt64(id);
				//os.writeString(client_->getEmail());
				//os.writeString(sender);
				//os.writeInt(net::kP2pAck_ConnTcp);
				//os.flushSize();
				//base::Utils::QueueSendRequest(socket, os, client_->getHandleComp(), ChatClient::kPeerConnKey);
				//return H_OK;
			//}
		//}
		//if (recverPublicIp) {
			//SockStream os;
			//os.writeInt(net::kCommandType_BuildPathAck);
			//os.writeInt(0);
			//os.writeInt64(id);
			//os.writeString(client_->getEmail());
			//os.writeString(sender);
			//os.writeInt(net::kP2pAck_ListenTcp);
			//os.writeInt(recverPublicIp);
			//os.writeShort(port_);
			//os.flushSize();
			//base::Utils::QueueSendRequest(socket, os, client_->getHandleComp(), ChatClient::kServerKey);
			//return H_OK;
		/*}*/
		//NAT
		return H_OK;
	}

	HERRCODE TcpPeerManager::onCmdBuildP2pPathAck(SOCKET socket, SockStream& is)
	{
   /*     int size;*/
		//int64_t id;
		//std::u16string sender;
		//std::u16string recver;
		//int type;
		//try {
			//size = is.getInt();
			//if (size != is.getSize())
				//return H_INVALID_PACKAGE;
			//id = is.getInt64();
			//sender = is.getString();
			//recver = is.getString();
			//if (recver != client_->getEmail())
				//return H_INVALID_PACKAGE;
			//type = is.getInt();
		//} catch (...) {
			//return H_INVALID_PACKAGE;
		//}

		//if (type == net::kP2pAck_ListenTcp) { // sender listen tcp
			//auto remoteIp = is.getInt();
			//auto remotePort = is.getShort();
			//sockaddr_in remoteAddr;
			//memset(&remoteAddr, 0, sizeof(sockaddr_in));
			//remoteAddr.sin_addr.S_un.S_addr = remoteIp;
			//remoteAddr.sin_family = AF_INET;
			//remoteAddr.sin_port = remotePort;
			//auto hr = connectToPeer(sender, remoteAddr.sin_addr, remoteAddr.sin_port, ChatClient::kPeerConnKey);
			//return hr;
		//} else if (type == net::kP2pAck_TcpHole) {
			//return H_OK;
		//} else if (type == net::kP2pAck_UdpHole) {
			//return H_OK;
		/*}*/
		return H_INVALID_PACKAGE;
	}

	HERRCODE TcpPeerManager::requestP2PConnect(SOCKET socket, const std::u16string& remote)
	{
   /*     auto id = time(NULL);*/
		//SockStream os;
		//os.writeInt(net::kCommandType_BuildPath);
		//os.writeInt(0);
		//os.writeInt64(id);
		//os.writeString(remote);
		//sockaddr_in localAddr;
		//int nameLen = sizeof(sockaddr_in);
		//if (getsockname(socket, (sockaddr*)&localAddr, &nameLen)) {
			//return H_NETWORK_ERROR;
		//}
		//os.writeInt(localAddr.sin_addr.S_un.S_addr);
		//os.writeShort(port_);
		//os.flushSize();
		/*base::Utils::QueueSendRequest(socket, os, client_->getHandleComp(), ChatClient::kServerKey);*/
		return H_OK;
	}
}
