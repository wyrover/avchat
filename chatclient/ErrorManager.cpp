#include "stdafx.h"
#include <algorithm>
#include "ErrorManager.h"
#include "../common/trace.h"
#include "MessageError.h"
#include "ChatClient.h"
#include "IChatClientController.h"
#include "MessageRequest.h"
#include "ChatError.h"

const static int kTimeOut = 5;
static std::recursive_mutex mutex;
namespace avc
{
	ErrorManager::ErrorManager(ChatClient* client)
	{
		client_ = client;
		quit_ = false;
		hQuitEvent_ = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	ErrorManager::~ErrorManager()
	{
	}

	void ErrorManager::addMessageRequest(int64_t id, const std::wstring& remote, int64_t timestamp)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		requests_.push_back(new MessageRequest(id, remote, timestamp));
	}

	void ErrorManager::confirmMessageRequest(int64_t id, const std::wstring& remote)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
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
		std::lock_guard<std::recursive_mutex> lock(mutex);
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
		HANDLE hTimer;
		LARGE_INTEGER li;
		hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
		const int nTimerUnitsPerSecond = 10000000;
		li.QuadPart = -(5 * nTimerUnitsPerSecond);

		HANDLE handles[] = { hTimer, hQuitEvent_ };
		while (!quit_) {
			SetWaitableTimer(hTimer, &li, 0,
				NULL, NULL, FALSE);
			auto result = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
			if (result == WAIT_OBJECT_0) {
				client_->queueCheckTimeoutTask();
			} else if (result == WAIT_OBJECT_0 + 1) {
				break;
			}
		}
		CloseHandle(hTimer);
	}

	void ErrorManager::quit()
	{
		quit_ = true;
		SetEvent(hQuitEvent_);
	}

	ChatError* ErrorManager::getChatError(ChatRequest* request, int64_t currTime)
	{
		switch (request->getType()) {
			case kChatError_Message:{
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