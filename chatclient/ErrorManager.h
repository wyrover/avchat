#pragma once

#include <mutex>
#include <string>
#include <stdint.h>
#include <vector>
namespace avc
{
	class ChatRequest;
	class ChatClient;
	class ChatError;
	class ErrorManager
	{
	public:
		ErrorManager(ChatClient* client);
		~ErrorManager();
		void addMessageRequest(int64_t id, const std::u16string& remote, int64_t timestamp);
		void confirmMessageRequest(int64_t id, const std::u16string& remote);
		void checkRequests();
		void threadFun();
		void quit();

	private:
		ChatError* getChatError(ChatRequest* request, int64_t currTime);
		std::vector<ChatRequest*> requests_;
		HANDLE hQuitEvent_;
		ChatClient* client_;
		volatile bool quit_;
	};
}
