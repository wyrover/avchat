#include "stdafx.h"
#include "../common/errcode.h"
#include "../common/ChatOverlappedData.h"
#include "../common/NetConstants.h"
#include "../common/SockStream.h"
#include "../common/trace.h"
#include "../common/Utils.h"
#include "ChatClient.h"


#define LOG_LOGIN 1
#define LOG_IOCP 0
const static int kCompKey = 233;
const static int kPort = 2333;
const static int kMaxAcceptRequest = 1000;

ChatClient::ChatClient()
{
	quit_ = false;
}

ChatClient::~ChatClient()
{
	quit(true);
	WSACleanup();
}

HERRCODE ChatClient::init(const std::wstring& serverAddr, int port)
{
	auto hr = initWinsock();
	if (hr != H_OK)
		return hr;

	sockaddr_in addr = { 0 };
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	IN_ADDR in_addr;
	InetPton(AF_INET, serverAddr.c_str(), &in_addr);
	addr.sin_addr = in_addr;
	sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (::connect(sock_, (const sockaddr*)&addr, sizeof(sockaddr_in)) != 0) {
		return H_NETWORK_ERROR;
	}
	return H_OK;
}

HERRCODE ChatClient::initWinsock()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
		return H_NETWORK_ERROR;
	return H_OK;
}

HERRCODE ChatClient::login(const std::wstring& username, const std::wstring& password)
{
	SockStream stream;
	stream.writeInt(net::kCommandType_Login);
	stream.writeInt(0);
	stream.writeString(username);
	stream.writeString(password);
	stream.flushSize();
	auto ret = ::send(sock_, stream.getBuf(), stream.getSize(), 0);
	if (ret == SOCKET_ERROR) {
		return H_NETWORK_ERROR;
	}
	buffer buf(100);
	int retLen = ::recv(sock_, buf.data(), buf.size(), 0);
	if (retLen) {
		SockStream ss(buf.data(), retLen);
		assert(net::kCommandType_LoginAck == ss.getInt());
		auto size = ss.getInt();
		auto ack = ss.getInt();
		if (ack) {
			hComp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
			if (hComp_ == NULL) {
				return H_FAILED;
			}
			HANDLE hComp2 = CreateIoCompletionPort((HANDLE)sock_, hComp_, kCompKey, 0);
			if (hComp2 != hComp_)
				return H_FAILED;
			username_ = username;

			return H_OK;
		} else {
			return H_AUTH_FAILED;
		}
	} else {
		return H_NETWORK_ERROR;
	}
}

HERRCODE ChatClient::sendMessage(const std::wstring& username, const std::wstring& message, time_t timestamp)
{
	if (message.find(L"<img") == -1) {
		SockStream stream;
		stream.writeInt(net::kCommandType_Message);
		stream.writeInt(0);
		stream.writeString(username_);
		stream.writeString(username);
		stream.writeInt64(timestamp);
		stream.writeString(message);
		stream.flushSize();
		queueSendRequest(sock_, stream);
	} else {
		ChatOverlappedData* ol = new ChatOverlappedData(net::kNetType_Send);
		ol->setCommandType(net::kCommandType_ImageMessage);
		ol->setMessage(message);
		PostQueuedCompletionStatus(hComp_, 0, kCompKey, ol);
	}
	return H_OK;
}

void ChatClient::threadFun(bool initRecv)
{
	if (initRecv) {
		base::Utils::QueueRecvCmdRequest(sock_);
	}
	while (!quit_) {
		if (!queueCompletionStatus()) {
			Sleep(100);
		}
	}
}

bool ChatClient::queueCompletionStatus()
{
	TRACE_IF(LOG_IOCP, "try to queue completion status\n");
	DWORD bytes;
	ULONG_PTR key; ChatOverlappedData* ol;
	if (!GetQueuedCompletionStatus(hComp_, &bytes, &key, (LPOVERLAPPED*)&ol, 0)) {
		TRACE_IF(LOG_IOCP, "no queued completion status\n");
		return false;
	}
	int type = ol->getNetType();
	TRACE_IF(LOG_IOCP, "get queued completion status type = %d\n", type);
	if (type == net::kNetType_Recv) {
		TRACE("get recv request result for ol %x\n", ol);
		onRecv(ol, bytes, key);
	} else if (type == net::kNetType_Send) {

	}
	return true;
}

void ChatClient::onRecv(ChatOverlappedData* ol, DWORD bytes, ULONG_PTR key)
{
	cmdCenter_.fill(ol->getSocket(), ol->getBuf().data(), bytes);
}

void ChatClient::setController(IChatClientController* controller)
{
	controller_ = controller;
	cmdCenter_.setController(controller);
}

void ChatClient::quit(bool wait)
{
	quit_ = true;
	if (wait) {
		for (auto& thread : threads_){
			thread.join();
		}
	}
}

std::vector<std::wstring> ChatClient::getUserList()
{
	return cmdCenter_.getUserList();
}

std::wstring ChatClient::getUsername()
{
	return username_;
}

void ChatClient::queueSendRequest(SOCKET socket, SockStream& stream)
{
	WSABUF wsaBuf;
	wsaBuf.buf = stream.getBuf();
	wsaBuf.len = stream.getSize();
	ChatOverlappedData* ol = new ChatOverlappedData(net::kNetType_Send);
	int ret = WSASend(socket, &wsaBuf, 1, NULL, 0, ol, NULL);
	int errcode = WSAGetLastError();
	if (ret != 0) {
		assert(errcode == WSA_IO_PENDING);
	}
}

void ChatClient::start()
{
	int threadCount = base::Utils::GetCpuCount() * 2;
	for (int i = 0; i < threadCount; ++i){
		threads_.push_back(std::thread(&ChatClient::threadFun, this, i == 0));
	}
}