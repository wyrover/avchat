#pragma once

#include <mutex>
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
		void addMessageRequest(int64_t id, const std::wstring& remote, int64_t timestamp);
		void confirmMessageRequest(int64_t id, const std::wstring& remote);
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