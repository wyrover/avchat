#include "stdafx.h"
#include <algorithm>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

#include "../common/trace.h"
#include "ErrorManager.h"
#include "MessageError.h"
#include "ChatClient.h"
#include "IChatClientController.h"
#include "MessageRequest.h"
#include "ChatError.h"

const static int kTimeOut = 5;

namespace avc
{
	ErrorManager::ErrorManager(ChatClient* client)
	{
		client_ = client;
		quit_ = false;
		kq_ = kqueue();
		bzero(pipeName_, sizeof(pipeName_));
	}

	ErrorManager::~ErrorManager()
	{
	}

	void ErrorManager::addMessageRequest(int64_t id, const std::u16string& remote, int64_t timestamp)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex_);
		requests_.push_back(new MessageRequest(id, remote, timestamp));
	}

	void ErrorManager::confirmMessageRequest(int64_t id, const std::u16string& remote)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex_);
		auto iter = std::remove_if(requests_.begin(), requests_.end(), [&id, &remote](ChatRequest* request) {
			if (request->getType() != kChatRequest_Message)
				return false;
			auto msg = dynamic_cast<MessageRequest*>(request);
			return (msg->getMessageId() == id && msg->getRemoteName() == remote);
		});
		if (iter != requests_.end()) {
			for (auto iter2 = iter; iter2 != requests_.end(); ++iter2) {
				delete *iter2;
			}
			requests_.erase(iter, requests_.end());
		}
	}

	void ErrorManager::checkRequests()
	{
		std::lock_guard<std::recursive_mutex> lock(mutex_);
		auto currTime = time(NULL);
		for (auto iter = requests_.begin(); iter != requests_.end();) {
			auto request = *iter;
			auto error = getChatError(request, currTime);
			if (error) {
				client_->controller()->onChatError(error);
				iter = requests_.erase(iter);
			} else {
				++iter;
			}
		}
	}

	void ErrorManager::threadFun()
	{
		strcpy(pipeName_, "/tmp/keke.XXXXX");
		if (mktemp(pipeName_) == NULL) {
			LOG_ERROR("cannot get temp file name");
			return;
		}
		umask(0);
		if (mknod(pipeName_, S_IFIFO | 0666, 0) < 0) {
			LOG_ERROR("cannot create pipe file");
			return;
		}
		int pipe = open(pipeName_, O_RDONLY | O_NONBLOCK);
		if (pipe < 0) {
			LOG_ERROR("cannot open pipe file");
			return;
		}
		struct kevent ev[2];
		EV_SET(&ev[0], pipe, EVFILT_READ, EV_ADD, 0, 0, 0);
		EV_SET(&ev[1], 1, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, 5, 0); 
		if (kevent(kq_, ev, 2, NULL, 0, NULL) < 0) {
			LOG_ERROR("add kevent failed");
			return;
		}
		while (!quit_) {
			struct kevent events[2];
			int count = kevent(kq_, NULL, 0, events, 2, 0);
			for (int i = 0; i < count; ++i) {
				if (events[i].filter == EVFILT_READ) {
					close(pipe);
				} else if (events[i].filter == EVFILT_TIMER) {
					checkRequests();
				}
			}
		}
	}

	void ErrorManager::quit()
	{
		quit_ = true;
		if (pipeName_[0]) {
			FILE* fp = fopen(pipeName_, "w");
			if (fp) {
				fputs("ke", fp);
				fclose(fp);

			}
		}
	}

	ChatError* ErrorManager::getChatError(ChatRequest* request, int64_t currTime)
	{
		switch (request->getType()) {
			case kChatRequest_Message:{
				auto sendTime = request->getTimestamp();
				if (currTime - sendTime > kTimeOut) {
					auto msgRequest = dynamic_cast<MessageRequest*>(request);
					auto msgError = new MessageError(msgRequest->getMessageId(), msgRequest->getRemoteName());
					return msgError;
				}
				break;
			}
		}
		return nullptr;
	}

}
