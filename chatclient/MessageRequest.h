#pragma once

#include "ChatRequest.h"
namespace avc
{
	class MessageRequest : public ChatRequest
	{
	public:
		MessageRequest(int64_t id, const std::wstring& remoteName, int64_t timestamp);
		~MessageRequest();
		int64_t getMessageId();
		std::wstring getRemoteName();
	private:
		int64_t id_;
		std::wstring remoteName_;
	};
}
