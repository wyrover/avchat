#include "stdafx.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../common/SockStream.h"
#include "../common/NetConstants.h"
#include "../common/trace.h"
#include "../common/Utils.h"
#include "CommandCenter.h"
#include "User.h"
#include "Client.h"
#include "unixserver.h"

#define LOG_SERVER 0
#define TEST_MODE 1

//fixme: should not call send multiple times?
// which could intepreter the package?
CommandCenter::CommandCenter(ChatServer* server)
{
	server_ = server;
	quit_ = false;
}

CommandCenter::~CommandCenter()
{
}

int CommandCenter::fill(SOCKET socket, char* inBuf, int inLen)
{
	if (inLen == 0)
		return 0;
	auto& cmdInfo = cmdMap_[socket];
	if (cmdInfo.fNeededLen == -1) {
		if (cmdInfo.fBuf.size() + inLen < 8) {
			cmdInfo.fBuf.append(inBuf, inLen);
			queueRecvCmdRequest(socket);
			return 0;
		} else {
			int needLen = 8 - cmdInfo.fBuf.size();
			cmdInfo.fBuf.append(inBuf, needLen);
			SockStream stream(cmdInfo.fBuf.data(), cmdInfo.fBuf.size());
			stream.getInt(); // type
			cmdInfo.fNeededLen = stream.getInt() - 8;
			inBuf += needLen;
			inLen -= needLen;
		}
	}

	if (inLen >= cmdInfo.fNeededLen) {
		cmdInfo.fBuf.append(inBuf, cmdInfo.fNeededLen);
		postCommand(socket, cmdInfo.fBuf);
		assert(cmdInfo.fBuf.empty());
		inBuf += cmdInfo.fNeededLen;
		inLen -= cmdInfo.fNeededLen;
		cmdInfo.fNeededLen = -1;
		if (inLen == 0) {
			queueRecvCmdRequest(socket);
			return 0;
		} else {
			return fill(socket, inBuf, inLen);
		}
	} else {
		cmdInfo.fBuf.append(inBuf, inLen);
		cmdInfo.fNeededLen -= inLen;
		queueRecvCmdRequest(socket);
	}
	return 0;
}

void CommandCenter::postCommand(SOCKET socket, buffer& cmdBuf)
{
	std::unique_lock<std::mutex> lock(mutex_);
	TRACE_IF(LOG_SERVER,"post command socket: %d, cmdBuf: %x, len: %d, after len = %d\n", socket, 
			cmdBuf.data(), cmdBuf.size(), cmdQueue_.size() + 1);
	cmdQueue_.push_back(CommandRequest(socket, cmdBuf));
	cv_.notify_one();
}

void CommandCenter::start()
{
	int threadCount = base::Utils::GetCpuCount() * 2;
	quit_ = false;
	for (int i = 0; i < threadCount; ++i) {
		threads_.push_back(std::thread(&CommandCenter::threadFun, this));
	}
}

void CommandCenter::threadFun()
{
	while (!quit_) {
		SOCKET s;
		buffer buf;
		{
			std::unique_lock<std::mutex> lock(mutex_);
			while (cmdQueue_.empty()) {
				cv_.wait(lock);
			}
			TRACE_IF(LOG_SERVER,"before pick the queue size = %d\n", cmdQueue_.size());
			s = cmdQueue_.front().fSock;
			buf.swap(cmdQueue_.front().fBuf);
			cmdQueue_.pop_front();
			TRACE_IF(LOG_SERVER,"pick command socket: %d, cmdBuf: %x, len: %d, after len = %d\n", s, 
					buf.data(), buf.size(), cmdQueue_.size());

		}
		handleCommand(s, buf);
	}
}

void CommandCenter::quit()
{
	quit_ = true;
	for (auto& thread : threads_) {
		thread.join();
	}
}

int CommandCenter::handleCommand(SOCKET socket, buffer& cmdBuf)
{
	TRACE_IF(LOG_SERVER,"handle command for socket: %d, cmdBuf: %x, size: %d\n",
			socket, cmdBuf.data(), cmdBuf.size());
	SockStream stream(cmdBuf.data(), cmdBuf.size());
	int type = stream.getInt();
	switch (type) {
	case net::kCommandType_BuildPathAck:
	{
		onCmdBuildP2pPathAck(socket, stream);
		break;
	}
	case net::kCommandType_BuildPath:
	{
		onCmdBuildP2pPath(socket, stream);
		break;
	}
	case net::kCommandType_FileTransferRequestAck:
	{
		onCmdFileTransferRequestAck(socket, stream);
		break;
	}
	case net::kCommandType_FileTransferRequest:
	{
		onCmdFileTransferRequest(socket, stream);
		break;
	}
	case net::kCommandType_Login:
	{
		onCmdLogin(socket, stream);
		break;
	}
	case net::kCommandType_Message:
	{

		onCmdMessage(socket, stream);
		break;
	}
	case net::kCommandType_FileExists:
	{
		auto size = stream.getInt();
		auto id = stream.getInt();
		auto hashList = stream.getStringVec();
		onCmdFileCheck(socket, id, hashList);
		break;
	}
	case net::kCommandType_FileUpload:
	{
		onCmdFileUpload(socket, stream);
		break;
	}
	case net::kCommandType_FileDownload:
	{
		onCmdFileDownload(socket, stream);
		break;
	}
	case net::kCommandType_Logout:
	{
		onCmdLogout(socket, stream);
		break;
	}
	default:
		break;
	}
	return 0;
}

void CommandCenter::onCmdLogin(SOCKET socket, SockStream& stream)
{
	stream.getInt();
	auto authType = stream.getInt();
	auto email = stream.getString();
	auto password = stream.getString();
	User user;
#if TEST_MODE
	if (true) {
#else
	if (user.login(authType, email, password) == H_OK) {
#endif
		auto client = new Client(user, socket);
		clientMan_.addClient(email, client);
		SockStream stream;
		stream.writeInt(net::kCommandType_LoginAck);
		stream.writeInt(0);
		stream.writeInt(net::kLoginAck_Succeeded);
		stream.writeInt(authType);
		stream.writeString(user.getAuthKey());
		stream.flushSize();
		queueSendRequest(socket, stream);
		TRACE_IF(LOG_SERVER, "send socket %d, login ack size %d\n", socket, stream.getSize());
		updateUserlist();
	} else {
		SockStream stream;
		stream.writeInt(net::kCommandType_LoginAck);
		stream.writeInt(0);
		stream.writeInt(net::kLoginAck_Failed);
		stream.flushSize();
		queueSendRequest(socket, stream);
	}
}

void CommandCenter::onCmdLogout(SOCKET socket, SockStream& stream)
{
	auto size = stream.getInt();
	auto email = stream.getString();
	clientMan_.removeClient(email);
	updateUserlist();
}

HERRCODE CommandCenter::onCmdMessage(SOCKET socket, SockStream& is)
{
	int size;
	std::u16string recver;
	std::u16string sender;
	int64_t timestamp;
	std::u16string message;
	try {
		size = is.getInt();
		if (size != is.getSize())
			return H_INVALID_PACKAGE;
		sender = is.getString();
		recver = is.getString();
		timestamp = is.getInt64();
		message = is.getString();
	} catch (std::out_of_range& e) {
		return H_INVALID_PACKAGE;
	}

	SockStream os;
	os.writeInt(net::kCommandType_Message);
	os.writeInt(0);
	os.writeString(sender);
	os.writeString(recver);
	os.writeInt64(timestamp);
	os.writeString(message);
	os.flushSize();
	if (recver == u"all") {
		std::lock_guard<std::recursive_mutex> lock(clientMan_.fMutex);
		for (auto& item : clientMan_.fClientMap) {
			//TRACE_IF(LOG_SERVER, "send message to all\n");
			//puts("send message to all\n");
			queueSendRequest(item.second->getSocket(), os);
		}
		{
			SockStream os;
			os.writeInt(net::kCommandType_MessageAck);
			os.writeInt(0); //dummy size
			os.writeInt64(timestamp);
			os.writeString(recver);
			os.flushSize();
			queueSendRequest(socket, os);
		}
	} else {
		SOCKET s;
		auto hr = clientMan_.getClientSocket(recver, &s);
		if (hr != H_OK)
			return hr;
		queueSendRequest(s, os);
	}
	return H_OK;
}

void CommandCenter::onCmdFileCheck(SOCKET socket, int id, const std::vector<std::u16string>& hashList)
{
	std::vector<std::u16string> urlList;
	for (auto hash : hashList) {
		std::u16string url;
		auto hr = fileMan_.getFileUrl(hash, &url);
		if (hr == H_OK && !url.empty()) {
			urlList.push_back(url);
		} else {
			urlList.push_back(u"");
		}
	}
	SockStream stream;
	stream.writeInt(net::kCommandType_FileExistsAck);
	stream.writeInt(0);
	stream.writeInt(id);
	stream.writeStringVec(urlList);
	stream.flushSize();
	queueSendRequest(socket, stream);
}

void CommandCenter::queueSendRequest(SOCKET socket, SockStream& stream)
{
	TRACE_IF(LOG_SERVER,"send socket %d stream size : %d, content: %s\n",
		   	socket, stream.getSize(), stream.toHexString().c_str());
	server_->queueSendRequest(socket, stream);
}

void CommandCenter::updateUserlist()
{
	SockStream stream;
	stream.writeInt(net::kCommandType_UserList);
	stream.writeInt(0);
	std::lock_guard<std::recursive_mutex> lock(clientMan_.fMutex);
	stream.writeInt(clientMan_.fClientMap.size());
	for (auto& item : clientMan_.fClientMap) {
		stream.writeString(item.second->getEmail());
	}
	stream.flushSize();
	for (auto& item : clientMan_.fClientMap) {
		SOCKET s = item.second->getSocket();
		TRACE_IF(LOG_SERVER, "send socket %d, userlist size %d\n", s, stream.getSize());
		queueSendRequest(s, stream);
	}
}

void CommandCenter::queueRecvCmdRequest(SOCKET socket)
{
}

void CommandCenter::onCmdFileUpload(SOCKET socket, SockStream& inStream)
{
	std::vector<std::u16string> urlList;
	auto size = inStream.getInt();
	int id = inStream.getInt();
	auto count = inStream.getInt();
	for (int i = 0; i < count; ++i) {
		auto fileType = inStream.getString();
		buffer buf;
		inStream.getBuffer(buf);
		std::u16string url;
		fileMan_.addFile(buf, fileType, &url);
		urlList.push_back(url);
	}
	SockStream outStream;
	outStream.writeInt(net::kCommandType_FileUploadAck);
	outStream.writeInt(0);
	outStream.writeInt(id);
	outStream.writeStringVec(urlList);
	outStream.flushSize();
	queueSendRequest(socket, outStream);
}

void CommandCenter::onCmdFileDownload(SOCKET socket, SockStream& stream)
{
	auto size = stream.getInt();
	auto id = stream.getInt();
	auto count = stream.getInt();
	SockStream outStream;
	outStream.writeInt(net::kCommandType_FileDownloadAck);
	outStream.writeInt(0);
	outStream.writeInt(id);
	outStream.writeInt(count); 
	for (int i = 0; i < count; ++i) {
		auto url = stream.getString();
		buffer outBuf;
		fileMan_.getFile(url, outBuf);
		outStream.writeBuffer(outBuf);
	}
	outStream.flushSize();
	queueSendRequest(socket, outStream);
}

void CommandCenter::onCmdFileTransferRequest(SOCKET socket, SockStream& stream)
{
	auto size = stream.getInt();
	auto recver = stream.getString();
	auto filename = stream.getString();
	auto filesize = stream.getInt();
	auto timestamp = stream.getInt64();
	std::u16string sender;
	auto hr = clientMan_.getEmailBySocket(socket, &sender);
	if (hr != H_OK)
		return;

	SockStream s;
	s.writeInt(net::kCommandType_FileTransferRequest);
	s.writeInt(0);
	s.writeString(sender);
	s.writeString(filename);
	s.writeInt(filesize);
	s.writeInt64(timestamp);
	s.flushSize();
	SOCKET clientSock;
	hr = clientMan_.getClientSocket(recver, &clientSock);
	if (hr != H_OK)
		return;
	queueSendRequest(clientSock, s);
}

void CommandCenter::onCmdFileTransferRequestAck(SOCKET socket, SockStream& stream)
{
	auto size = stream.getInt();
	auto recver = stream.getString();
	auto timestamp = stream.getInt64();
	auto isRecv = stream.getBool();
	std::u16string sender;
	auto hr = clientMan_.getEmailBySocket(socket, &sender);
	if (hr != H_OK)
		return;

	SockStream s;
	s.writeInt(net::kCommandType_FileTransferRequestAck);
	s.writeInt(0);
	s.writeString(sender);
	s.writeInt64(timestamp);
	if (!isRecv) {
		s.writeBool(false);
	} else {
		s.writeBool(true);
	}
	s.flushSize();
	SOCKET clientSock;
	hr = clientMan_.getClientSocket(recver, &clientSock);
	if (hr != H_OK)
		return;
	queueSendRequest(clientSock, s);
}

HERRCODE CommandCenter::onCmdBuildP2pPath(SOCKET socket, SockStream& is)
{
	int size;
	int64_t id;
	std::u16string sender;
	std::u16string authKey;
	std::u16string recver;
	int localIp;
	short localPort;
	try {
		size = is.getInt();
		if (size != is.getSize()) {
			return H_NETWORK_ERROR;
		}
		id = is.getInt64();
		sender = is.getString();
		authKey = is.getString();
		recver = is.getString();
		localIp = is.getInt();
		localPort = is.getInt();
	} catch (...) {
		return H_NETWORK_ERROR;
	}

	User user;
	if (!user.login(net::kLoginType_Auto, sender, authKey)) {
		return H_AUTH_FAILED;
	}

	sockaddr_in publicAddr;
	socklen_t nameLen = sizeof(sockaddr_in);
	if (getpeername(socket, (sockaddr*)&publicAddr, &nameLen)) {
		return H_NETWORK_ERROR;
	}

	SOCKET recvSock;
	auto hr = clientMan_.getClientSocket(recver, &recvSock);
	if (hr != H_OK)
		return H_FAILED;

	SockStream os;
	os.writeInt(net::kCommandType_BuildPath);
	os.writeInt(0);
	os.writeInt64(id);
	os.writeString(sender);
	os.writeInt(localIp);
	os.writeShort(localPort);
	os.writeInt(publicAddr.sin_addr.s_addr);
	os.writeShort(publicAddr.sin_port);
	os.flushSize();
	queueSendRequest(recvSock, os);
}

HERRCODE CommandCenter::onCmdBuildP2pPathAck(SOCKET socket, SockStream& is)
{
	int size;
	int64_t id;
	std::u16string sender;
	std::u16string recver;
	int type;
	try {
		size = is.getInt();
		if (size != is.getSize())
			return H_NETWORK_ERROR;
		id = is.getInt64();
		sender = is.getString();
		recver = is.getString();
		type = is.getInt();
	} catch (...) {
		return H_NETWORK_ERROR;
	}

	if (type == net::kP2pAck_ListenTcp) { // sender listen tcp
		SOCKET recvSock;
		auto hr = clientMan_.getClientSocket(recver, &recvSock);
		if (hr != H_OK)
			return H_NETWORK_ERROR;
		//base::Utils::QueueSendRequest(recvSock, is, server_->getCompletePortHandle(), ChatServer::kTcpCompKey);
		return H_OK;
	} else if (type == net::kP2pAck_TcpHole) {
		return H_OK;
	} else if (type == net::kP2pAck_UdpHole) {
		return H_OK;
	}
	return H_FAILED;
}
