#include "stdafx.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>
#include <sstream>
#include "../common/Utils.h"
#include "../common/NetUtils.h"
#include "../common/NetConstants.h"
#include "../common/trace.h"
#include "TcpPeerManager.h"
#include "ChatClient.h"
#include "CommandCenter.h"

#define LOG_KQUEUE 0

namespace avc
{
	TcpPeerManager::TcpPeerManager(ChatClient* client)
	{
		client_ = client;
		kq_ = -1;
		sock_ = -1;
		port_ = -1;
		quit_ = false;
	}

	TcpPeerManager::~TcpPeerManager()
	{
	}

	HERRCODE TcpPeerManager::initSock()
	{
		sock_ = socket(AF_INET, SOCK_STREAM, 0);
		if (sock_ == -1)
			return H_NETWORK_ERROR;

		if (!base::NetUtils::BindSocket(sock_, "", "0"))
			return H_NETWORK_ERROR;

		sockaddr_in addr;
		socklen_t len;
		if (getsockname(sock_, (sockaddr*)&addr, &len) < 0) {
			close(sock_);
			sock_ = -1;
			return H_NETWORK_ERROR;
		}
		port_ = addr.sin_port;

		if (base::NetUtils::MakeSocketNonBlocking(sock_))
			return H_NETWORK_ERROR;

		auto rc = listen(sock_, SOMAXCONN);
		if (rc != 0) {
			return H_NETWORK_ERROR;
		}

		kq_ = kqueue();
		if (kq_ == -1)
			return H_NETWORK_ERROR;

		struct kevent ev;
		EV_SET(&ev, sock_, EVFILT_READ, EV_ADD, 0, 0, 0);
		if (kevent(kq_, &ev, 1, NULL, 0, NULL)  < 0) {
			return H_NETWORK_ERROR;
		}

		threads_.push_back(std::thread(&TcpPeerManager::netWorkerFunc, this));
		threads_.push_back(std::thread(&TcpPeerManager::timerCheckFunc, this));
		return H_OK;
	}
	
	void TcpPeerManager::netWorkerFunc()
	{
		TRACE_IF(LOG_KQUEUE, "client peermanager::run %d\n", std::this_thread::get_id());
		std::vector<struct kevent> events(500);
		struct timespec tmout = { 1, 0 };
		while (!quit_) {
			int eventCount = kevent(kq_, NULL, 0, events.data(), events.size(), &tmout);
			if (eventCount > 0) {
				for (int i = 0; i < eventCount; ++i) {
					printf("kevent %d event\n", eventCount);
					if (events[i].flags & EV_EOF) {
						std::stringstream idStr;
						idStr << std::this_thread::get_id();
						fprintf(stdout, "(%s)kqueue client socket done socket %x\n",
								idStr.str().c_str(), (unsigned int)events[i].ident);
						struct kevent ev;
						EV_SET(&ev, events[i].ident, EVFILT_READ, EV_DELETE, 0, 0, 0);
						if (kevent(kq_, &ev, 1, NULL, 0, NULL) < 0) {
							perror("remove clientfd from kqueue failed\n");
						} 
						close(events[i].ident);
						continue;
					}
					if (events[i].flags & EV_ERROR) {
						perror("kqueue error\n");
						close(events[i].ident);
						continue;
					}

					if (events[i].filter == EVFILT_READ)  {
						if (events[i].ident == sock_) {
							acceptConnections(events[i].ident);
						} else {
							puts("handle filt_read");
							size_t len = events[i].data;
							buffer buf(len);
							bool done = false;
							auto rc = recv(events[i].ident, buf.data(), len, 0);
							if (rc == -1) {
								if (errno != EAGAIN) {
									perror("recv error close socket");
									close(events[i].ident);
								} 
								break;
							}
							client_->cmdCenter().fill(events[i].ident, buf.data(), rc);
						}
					} else if (events[i].filter == EVFILT_WRITE) {
						puts("handle filt_write");
						if (!client_->sendRequest(events[i].ident, events[i].data)) {
							perror("send request failed\n");
							close(events[i].ident);
						}
					}
				}
			}
		}
	}

	void TcpPeerManager::timerCheckFunc()
	{
	}

	int TcpPeerManager::acceptConnections(SOCKET sock)
	{
		while (true) {
			sockaddr_in remoteAddr;
			socklen_t addrLen = sizeof(sockaddr_in);
			auto clientfd = accept(sock, (sockaddr*)&remoteAddr, &addrLen);
			if (clientfd != -1) {
				base::NetUtils::MakeSocketNonBlocking(clientfd);
				acceptedRequest_++;
#if LOG_LOGIN
				char str[20];
				inet_ntop(AF_INET, &remoteAddr.sin_addr, str, 20);
				TRACE_IF(LOG_LOGIN, "%d recv connection from %s, thread id = %d\n",
						(int)acceptedRequest_, str, std::this_thread::get_id());
#endif
				struct kevent ev;
				EV_SET(&ev, clientfd, EVFILT_READ, EV_ADD, 0, 0, 0);
				if (kevent(kq_, &ev, 1, NULL, 0, NULL) < 0) {
					perror("set clientfd kqueue read failed\n");
					close(clientfd);
					return -1;
				}
			} else {
				if (errno != EAGAIN && errno != EWOULDBLOCK)
					perror("accept error\n");
				return -1;
			}
		}
		return 0;
	}

	bool TcpPeerManager::quit()
	{
		quit_ = true;
		return true;
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

	HERRCODE TcpPeerManager::connectToPeer(const std::u16string& remoteName, const sockaddr_in& remoteAddr,
			const std::u16string& authKey)
	{
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET)
			return H_NETWORK_ERROR;
		if (!base::NetUtils::MakeSocketNonBlocking(sock))
			return H_NETWORK_ERROR;
		auto err = connect(sock, (const sockaddr*)&remoteName, sizeof(sockaddr_in));
		if (err == 0) {
			struct kevent ev;
			EV_SET(&ev, sock_, EVFILT_READ, EV_ADD, 0, 0, 0);
			if (kevent(kq_, &ev, 1, NULL, 0, 0) < 0) {
				syslog(LOG_ERR, "kqeueue change client socket failed\n");
				close(sock_);
				sock_ = -1;
				return H_NETWORK_ERROR;
			}
			syslog(LOG_DEBUG, "kqueue add socket %d filt-read succeeded\n", sock_);
		} else if (err == -1 && errno != EINPROGRESS) {
			syslog(LOG_ERR, "connect errrno %d, %s\n", errno, strerror(errno));
			return H_NETWORK_ERROR;
		} else {
			if (kq_ == -1) {
				kq_ = kqueue();
				if (kq_ == -1) {
					syslog(LOG_ERR, "kqueue create failed\n");
					return H_SERVER_ERROR;
				}
			}
			struct kevent ev;
			EV_SET(&ev, sock_, EVFILT_WRITE, EV_ADD, 0, 0, 0);
			if (kevent(kq_, &ev, 1, NULL, 0, 0) < 0) {
				syslog(LOG_ERR, "kqueue %d add socket %d filt-write failed errno: %d, %s\n",
						kq_, sock_, errno, strerror(errno));
				close(sock_);
				sock_ = -1;
				return H_NETWORK_ERROR;
			} else {
				syslog(LOG_DEBUG, "kqueue %d add socket %d filt-write succeeded\n",
						kq_, sock_);
			}
		}
		return H_OK;
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
//		auto ackStr = base::Utils::XorString(remoteStr, kPeerSyncString);
//		sockMap_[remoteName] = socket;
//		SockStream  os;
//		os.writeInt(net::kCommandType_PeerSyncAck);
//		os.writeInt(0);
//		os.writeString(kPeerSyncString);
//		os.writeString(client_->getEmail());
//		os.writeString(ackStr);
//		os.flushSize();
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
