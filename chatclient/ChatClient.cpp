#include <arpa/inet.h>
#include "ImageMessageForSend.h"
#include <assert.h>
#include <chatclient/ChatClient.h>
#include <chatclient/LoginError.h>
#include <common/buffer.h>
#include <common/FileUtils.h>
#include <common/NetConstants.h>
#include <common/SockStream.h>
#include <common/StringUtils.h>
#include <common/trace.h>
#include <common/Utils.h>
#include <netinet/in.h>
#include <sys/_endian.h>
#include <sys/_types/_size_t.h>
#include <sys/_types/_time_t.h>
#include <sys/_types/_timespec.h>
#include <sys/errno.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <syslog.h>
#include <unistd.h>
#include <__mutex_base>
#include <cstdio>
#include <cstring>
#include <sstream>

#define LOG_LOGIN 1
#define LOG_KQUEUE 0

const static int kMaxEventsCount = 1000;

namespace avc
{
ChatClient::ChatClient()
: msgMan_(this), peerMan_(this)
{
        connected_ = false;
	quit_ = false;
	sock_ = -1;
	kq_ = -1;
	controller_ = NULL;
	serverPort_ = -1;
	ownkq_ = false;
}

ChatClient::~ChatClient()
{
	quit(true);
	if (sock_ != -1)
		close(sock_);
	if (kq_ != -1 && ownkq_)
		close(kq_);
}

HERRCODE ChatClient::init(const std::u16string& serverAddr, int port, int kq /* = -1*/)
{
	kq_ = kq;
	if (kq_ == -1) {
		kq_ = kqueue();
		if (kq_ == -1) {
			LOG_ERROR("%s", "kqueue create failed\n");
			return H_SERVER_ERROR;
		}
		ownkq_ = true;
	}

	serverAddr_ = serverAddr;
	serverPort_ = port;
	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
		return H_FAILED;
	rl.rlim_cur = 10240;
	if (setrlimit(RLIMIT_NOFILE, &rl) < 0)
		return H_FAILED;
	return H_OK;
}

HERRCODE ChatClient::autoLogin(const std::u16string& username, const std::u16string& token)
{
	return loginImpl(net::kLoginType_Auto, username, token);
}

HERRCODE ChatClient::login(const std::u16string& username, const std::u16string& password)
{
	return loginImpl(net::kLoginType_Normal, username, password);
}

HERRCODE ChatClient::queueSendRequest(SOCKET sock, SockStream& stream)
{
	std::lock_guard<std::mutex> guard(sendMutex_);
	auto& buffer = sendBuf_[sock];
	if (!buffer.empty()) {
		buffer.append(stream.getBuf(), stream.getSize());
		return H_OK;
	} else {
		int rc = send(sock, stream.getBuf(), stream.getSize(), 0);
		if (rc > 0) {
			if (rc == stream.getSize()) {
				TRACE("send request directly done %d\n", stream.getSize());
				return H_OK;
			} else {
				buffer.append(stream.getBuf() + rc, stream.getSize() - rc);
				if (!setWatchOut(sock, true))
					return H_FAILED;
				else
					return H_OK;
			}
		} else {
			if (errno != EAGAIN) {
				//fixme: need handle ENOTBUFS/EPIPE ?
				LOG_ERROR("send failed errno: %d, %s\n", errno, strerror(errno));
				return H_FAILED;
			}
			buffer.append(stream.getBuf(), stream.getSize());
			if (!setWatchOut(sock, true))
				return H_FAILED;
			else
				return H_OK;
		}
	}
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
	} else {
		ImageMessageForSend* msg = new ImageMessageForSend;
		msg->setRawMessage(message, username, timestamp);

		SockStream cmd;
		cmd.writeInt(net::kCommandType_ImageMessage);
		cmd.writeInt(0); //dummy size
		cmd.writeInt64((int64_t)msg);
		cmd.flushSize();
		cmdCenter_.postCommand(sock_, *(buffer*)&cmd);
	}
	msgMan_.addMessageRequest(timestamp, username, timestamp);
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

void ChatClient::threadFun()
{
	TRACE_IF(LOG_KQUEUE, "chatserver::run %d\n", std::this_thread::get_id());
	std::vector<struct kevent> events;
	events.resize(kMaxEventsCount);
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
						LOG_ERROR("remove clientfd from kqueue failed\n");
					}
					close(events[i].ident);
					continue;
				}
				if (events[i].flags & EV_ERROR) {
					LOG_ERROR("kqueue error\n");
					close(events[i].ident);
					continue;
				}

				if (events[i].filter == EVFILT_READ)  {
					puts("handle filt_read");
					size_t len = events[i].data;
					buffer buf(len);
					bool done = false;
					auto rc = recv(events[i].ident, buf.data(), len, 0);
					if (rc == -1) {
						if (errno != EAGAIN) {
							LOG_ERROR("recv error close socket");
							close(events[i].ident);
						}
						break;
					}
					cmdCenter_.fill(events[i].ident, buf.data(), rc);
				} else if (events[i].filter == EVFILT_WRITE) {
                                        puts("handle filt_write");
                                        if (!connected_) {
                                            handleConnect();
                                        } else {
                                                if (!sendRequest(events[i].ident, events[i].data)) {
                                                        LOG_ERROR("send request failed\n");
                                                        close(events[i].ident);
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
	int threadCount = 1;
	for (int i = 0; i < threadCount; ++i){
                threads_.push_back(std::thread(&ChatClient::threadFun, this));
	}
	threads_.push_back(std::thread(&ErrorManager::threadFun, &msgMan_));
	cmdCenter_.start();
}

void ChatClient::setImageCacheDir(const std::u16string& filePath)
{
	imageCache_ = filePath + u"/" + email_ + u"/";
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

	auto rc = initSock();
	if (rc != H_OK)
		return rc;
	sockaddr_in addr = { 0 };
	addr.sin_port = htons(serverPort_);
	addr.sin_family = AF_INET;
	in_addr iaddr;
	auto uServerAddr = su::u16to8(serverAddr_);
	if (inet_pton(AF_INET, uServerAddr.c_str(), &iaddr) != 1) {
		LOG_ERROR("network address translate failed\n");
		return H_NETWORK_ERROR;
	}
	addr.sin_addr = iaddr;
	auto err = connect(sock_, (const sockaddr*)&addr, sizeof(sockaddr_in));
	if (err == 0) {
		connected_ = true;
		struct kevent ev;
		EV_SET(&ev, sock_, EVFILT_READ, EV_ADD, 0, 0, 0);
		if (kevent(kq_, &ev, 1, NULL, 0, 0) < 0) {
			LOG_ERROR("kqeueue change client socket failed\n");
			close(sock_);
			sock_ = -1;
			return H_NETWORK_ERROR;
		}
		syslog(LOG_DEBUG, "kqueue add socket %d filt-read succeeded\n", sock_);
	} else if (err == -1 && errno != EINPROGRESS) {
		LOG_ERROR("connect errrno %d, %s\n", errno, strerror(errno));
		return H_NETWORK_ERROR;
	} else {
		struct kevent ev;
		EV_SET(&ev, sock_, EVFILT_WRITE, EV_ADD, 0, 0, 0);
		if (kevent(kq_, &ev, 1, NULL, 0, 0) < 0) {
			LOG_ERROR("kqueue %d add socket %d filt-write failed errno: %d, %s\n",
					kq_, sock_, errno, strerror(errno));
			close(sock_);
			sock_ = -1;
			return H_NETWORK_ERROR;
		} else {
			syslog(LOG_DEBUG, "kqueue %d add socket %d filt-write succeeded\n",
					kq_, sock_);
		}
	}
	start();
	return H_OK;
}

HERRCODE ChatClient::handleConnect()
{
	int optvalue = -1;
	socklen_t optlen = sizeof(int);
	if (getsockopt(sock_, SOL_SOCKET, SO_ERROR, &optvalue, &optlen) != 0 || optvalue != 0) {
		connected_ = false;
		controller()->onChatError(new LoginError());
		return H_NETWORK_ERROR;
	}

	connected_ = true;
	struct kevent ev;
	EV_SET(&ev, sock_, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
	if (kevent(kq_, &ev, 1, NULL, 0, 0) < 0) {
		LOG_ERROR("kqeueue change client socket %d, failed, errno: %d, %s\n",
				sock_, errno, strerror(errno));
		close(sock_);
		sock_ = -1;
		return H_NETWORK_ERROR;
	}

	EV_SET(&ev, sock_, EVFILT_READ, EV_ADD, 0, 0, 0);
	if (kevent(kq_, &ev, 1, NULL, 0, 0) < 0) {
		LOG_ERROR("kqeueue change client socket failed\n");
		close(sock_);
		sock_ = -1;
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
	if (sock_ == -1) {
		LOG_ERROR("cannot create socket error: %d, %s\n", errno, strerror(errno));
		return H_NETWORK_ERROR;
	}
	syslog(LOG_DEBUG, "created socket %d\n", sock_);
	if (base::Utils::MakeSocketNonBlocking(sock_) < 0) {
		LOG_ERROR("cannot make socket nonblocking\n");
		return H_NETWORK_ERROR;
	}
	return H_OK;
}

TcpPeerManager& ChatClient::peerMan()
{
	return peerMan_;
}

bool ChatClient::sendRequest(SOCKET sock, int len)
{
	std::lock_guard<std::mutex> guard(sendMutex_);
	auto& buf = sendBuf_[sock];
	assert(!buf.empty());
	if (len > buf.size())
		len = buf.size();
	int rc = send(sock, buf.data(), len, 0);
	if (rc > 0) {
		if (rc == buf.size()) {
			buf.clear();
			if (!setWatchOut(sock, false))
				return false;
			return true;
		} else {
			auto rptr = buf.data() + rc;
			auto rlen = buf.size() - rc;
			buf.detach();
			buf.assign(rptr, rlen);
			return true;
		}
	} else {
		if (errno != EAGAIN) {
			//fixme: need handle ENOTBUFS/EPIPE ?
			LOG_ERROR("send failed errno: %d, %s\n", errno, strerror(errno));
			return false;
		}
		return true;
	}
}

bool ChatClient::setWatchOut(SOCKET sock, bool watch)
{
	struct kevent ev;
	EV_SET(&ev, sock, EVFILT_WRITE,
			(watch ? EV_ADD : EV_DELETE), 0, 0, 0);
	if (kevent(kq_, &ev, 1, NULL, 0, NULL) < 0) {
		LOG_ERROR("set clientfd kqueue write failed\n");
		return false;
	}
	return true;
}

CommandCenter& ChatClient::cmdCenter()
{
	return cmdCenter_;
}

}

