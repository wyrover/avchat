#pragma once

#include "ChatRequest.h"
#include <string>
namespace avc
{
	class MessageRequest : public ChatRequest
	{
	public:
		MessageRequest(int64_t id, const std::u16string& remoteName, int64_t timestamp);
		~MessageRequest();
		int64_t getMessageId();
		std::u16string getRemoteName();
	private:
		int64_t id_;
		std::u16string remoteName_;
	};
}
