#include "stdafx.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>
#include <assert.h>
#include "../common/errcode.h"
#include "../common/NetConstants.h"
#include "../common/SockStream.h"
#include "../common/trace.h"
#include "../common/Utils.h"
#include "../common/FileUtils.h"
#include "../common/StringUtils.h"
#include "LoginError.h"
#include "ImageMessageForSend.h"
#include "UtilsTest.h"
#include "ChatClient.h"

#define LOG_LOGIN 1
#define LOG_EPOLL 0

const static int kMaxEventsCount = 1000;

namespace avc
{
	ChatClient::ChatClient()
		: msgMan_(this), peerMan_(this)
	{
		quit_ = false;
		sock_ = -1;
		listenSock_ = -1;
		epfd_ = -1;
		controller_ = NULL;
		serverPort_ = -1;
	}

	ChatClient::~ChatClient()
	{
		quit(true);
	}

	HERRCODE ChatClient::init(const std::u16string& serverAddr, int port)
	{
		serverAddr_ = serverAddr;
		serverPort_ = port;
		return H_OK;
		//		return initSock();
	}

	HERRCODE ChatClient::autoLogin(const std::u16string& username, const std::u16string& token)
	{
		return loginImpl(net::kLoginType_Auto, username, token);
	}

	HERRCODE ChatClient::login(const std::u16string& username, const std::u16string& password)
	{
		return loginImpl(net::kLoginType_Normal, username, password);
	}

	void ChatClient::queueSendRequest(SOCKET socket, SockStream& stream) 
	{
		send(socket, stream.getBuf(), stream.getSize(), 0);
	}

	HERRCODE ChatClient::sendMessage(const std::u16string& username, const std::u16string& message, time_t timestamp)
	{
		if (message.find(u"<img") == -1) {
			SockStream stream;
			stream.writeInt(net::kCommandType_Message);
			stream.writeInt(0);
			stream.writeString(email_);
			stream.writeString(username);
			stream.writeInt64(timestamp);
			stream.writeString(message);
			stream.flushSize();
			queueSendRequest(sock_, stream);
			msgMan_.addMessageRequest(timestamp, username, timestamp);
		} else {
			/*         ChatOverlappedData* ol = new ChatOverlappedData(net::kAction_SendMessage);*/
			//ImageMessageForSend* msg = new ImageMessageForSend;
			//msg->setRawMessage(message, username, timestamp);
			//ol->setProp((int)msg);
			/*PostQueuedCompletionStatus(hComp_, 0, kServerKey, ol);*/
		}
		return H_OK;
	}

	HERRCODE ChatClient::sendFileTransferRequest(const std::u16string& email, const RequestFilesInfo& fileInfo, time_t timestamp)
	{
		SockStream os;
		os.writeInt(net::kCommandType_FileTransferRequest);
		os.writeInt(0);
		os.writeString(email);
		os.writeString(fileInfo.fileName);
		os.writeInt(fileInfo.fileSize);
		os.writeInt64(timestamp);
		os.flushSize();
		queueSendRequest(sock_, os);
		return H_OK;
	}

	HERRCODE ChatClient::confirmFileTransferRequest(const std::u16string& username, time_t timestamp,
			bool recv, const std::u16string& savePath)
	{
		if (!recv) {
			SockStream os;
			os.writeInt(net::kCommandType_FileTransferRequestAck);
			os.writeInt(0);
			os.writeString(username);
			os.writeInt64(timestamp);
			os.writeBool(false);
			os.flushSize();
			queueSendRequest(sock_, os);
		} else {
			SockStream os;
			os.writeInt(net::kCommandType_FileTransferRequestAck);
			os.writeInt(0);
			os.writeString(username);
			os.writeInt64(timestamp);
			os.writeBool(true);
			os.flushSize();
			queueSendRequest(sock_, os);
		}
		return H_OK;
	}

	HERRCODE ChatClient::logout()
	{
		SockStream stream;
		stream.writeInt(net::kCommandType_Logout);
		stream.writeInt(0);
		stream.writeString(email_);
		stream.flushSize();
		auto ret = ::send(sock_, stream.getBuf(), stream.getSize(), 0);
		if (ret == -1) {
			return H_NETWORK_ERROR;
		}
		quit(true);
		return H_OK;
	}

	void ChatClient::threadFun(bool initRecv)
	{
		TRACE_IF(LOG_EPOLL, "chatclient::run %d\n", std::this_thread::get_id());
		while (!quit_) {
			int eventCount = epoll_wait(epfd_, events_.data(), kMaxEventsCount, 100);
			if (eventCount > 0) {
				for (int i = 0; i < eventCount; ++i) {
//                                        if (events_[i].events & EPOLLERR) {
//						perror("epoll error\n");
//						close(events_[i].data.fd);
//						continue;
//					}
					if (events_[i].data.fd == listenSock_) {
						while (true) {
							TRACE("event events flags = %d\n", events_[i].events);
							sockaddr_in remoteAddr;
							socklen_t addrLen = sizeof(sockaddr_in);
							auto clientfd = accept(listenSock_, (sockaddr*)&remoteAddr, &addrLen);
							if (clientfd != -1) {
								auto rc = base::Utils::MakeSocketNonBlocking(clientfd);
								acceptedRequest_++;
#if LOG_LOGIN
								char str[20];
								inet_ntop(AF_INET, &remoteAddr.sin_addr, str, 20);
								TRACE_IF(LOG_LOGIN, "%d recv connection from %s, thread id = %d\n",
										(int)acceptedRequest_, str, std::this_thread::get_id());
#endif

								epoll_event ev;
								ev.events = EPOLLIN | EPOLLET;
								ev.data.fd = clientfd;
								epoll_ctl(epfd_, EPOLL_CTL_ADD, clientfd, &ev);
							} else {
								if (errno != EAGAIN && errno != EWOULDBLOCK)
									perror("accept error\n");
								break;
							}
						}
					} else {
						if (events_[i].events & EPOLLOUT) {
							handleConnect();
						} else if (events_[i].events & EPOLLIN) {
							buffer buf(1024);
							size_t len = 1024;
							bool done = false;
							while (true) {
								auto rc = recv(events_[i].data.fd, buf.data(), len, 0);
								if (rc == -1) {
									if (errno != EAGAIN) {
										perror("read");
										done = true;
									}
									break;
								} else if (rc == 0) {
									done = true;
									break;
								}
								cmdCenter_.fill(events_[i].data.fd, buf.data(), rc);
							}
							if (done) {
								close(events_[i].data.fd);
							}
						}
					}
				}
			}
		}
	}

	void ChatClient::setController(IChatClientController* controller)
	{
		controller_ = controller;
		cmdCenter_.setController(this, controller);
	}

	void ChatClient::quit(bool wait)
	{
		quit_ = true;
		msgMan_.quit();
		for (auto& thread : threads_){
			thread.join();
		}
	}

	std::vector<std::u16string> ChatClient::getUserList()
	{
		return cmdCenter_.getUserList();
	}

	std::u16string ChatClient::getEmail()
	{
		return email_;
	}

	void ChatClient::start()
	{
                int threadCount = base::Utils::GetCpuCount() * 2;
		for (int i = 0; i < threadCount; ++i){
			threads_.push_back(std::thread(&ChatClient::threadFun, this, i == 0));
		}
		threads_.push_back(std::thread(&ErrorManager::threadFun, msgMan_));
	}

	void ChatClient::setImageCacheDir(const std::u16string& filePath)
	{
		imageCache_ = filePath + email_ + u"\\";
		FileUtils::MkDirs(su::u16to8(imageCache_));
	}

	std::u16string ChatClient::getImageDir()
	{
		return imageCache_;
	}

	HERRCODE ChatClient::loginImpl(int type, const std::u16string& username, const std::u16string& credential)
	{
		loginRequest_.type = type;
		loginRequest_.username = username;
                loginRequest_.password = credential;
                email_ = username;

		initSock();
		sockaddr_in addr = { 0 };
		addr.sin_port = htons(serverPort_);
		addr.sin_family = AF_INET;
		in_addr iaddr;
		auto uServerAddr = su::u16to8(serverAddr_);
		if (inet_pton(AF_INET, uServerAddr.c_str(), &iaddr) != 1)
			return H_NETWORK_ERROR;
		addr.sin_addr = iaddr;
		auto err = connect(sock_, (const sockaddr*)&addr, sizeof(sockaddr_in));
		if (err == -1) {
			if (errno == EINPROGRESS) {
				if (epfd_ == -1) {
					epfd_ = epoll_create(kMaxEventsCount);
					if (epfd_ == -1)
						return H_SERVER_ERROR;
				}
				events_.resize(kMaxEventsCount);
				epoll_event ev;
				ev.events = EPOLLOUT | EPOLLIN | EPOLLET;
				ev.data.fd = sock_;
				if (epoll_ctl(epfd_, EPOLL_CTL_ADD, sock_, &ev) < 0)
					return H_SERVER_ERROR;
				start();
			} else {
				openlog("chatclient", LOG_PID|LOG_CONS, LOG_USER);
				syslog(LOG_ERR, "connect errrno %d\n", errno);
				closelog();
				return H_NETWORK_ERROR;
			}
		} else {
			start();
		}
		return H_OK;
	}

	HERRCODE ChatClient::handleConnect()
	{
		int optvalue = -1;
		socklen_t optlen = sizeof(int);
		if (getsockopt(sock_, SOL_SOCKET, SO_ERROR, &optvalue, &optlen) != 0 || optvalue != 0) {
			controller()->onChatError(new LoginError());
			return H_NETWORK_ERROR;
		}

		epoll_event ev;
		ev.events = EPOLLIN | EPOLLET;
		ev.data.fd = sock_;
		if (epoll_ctl(epfd_, EPOLL_CTL_MOD, sock_, &ev) != 0) {
			controller()->onChatError(new LoginError());
			return H_NETWORK_ERROR;
		}

		SockStream stream;
		stream.writeInt(net::kCommandType_Login);
		stream.writeInt(0);
		stream.writeInt(loginRequest_.type);
		stream.writeString(loginRequest_.username);
		stream.writeString(loginRequest_.password);
		stream.flushSize();
		queueSendRequest(sock_, stream);
		return H_OK;
	}

	ErrorManager& ChatClient::messageMan()
	{
		return msgMan_;
	}

	IChatClientController* ChatClient::controller()
	{
		return controller_;
	}

	void ChatClient::queueCheckTimeoutTask()
	{
		//PostQueuedCompletionStatus(hComp_, 0, kTimerKey, NULL);
	}

	HERRCODE ChatClient::initSock()
	{
		sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock_ == -1)
			return H_NETWORK_ERROR;
		if (base::Utils::MakeSocketNonBlocking(sock_) < 0)
			return H_NETWORK_ERROR;
		return H_OK;
		return H_OK;
	}

	TcpPeerManager& ChatClient::peerMan()
	{
		return peerMan_;
	}
}
