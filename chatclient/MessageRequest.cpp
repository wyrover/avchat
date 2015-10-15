#include "stdafx.h"
#include "MessageRequest.h"

namespace avc
{
	MessageRequest::MessageRequest(int64_t id, const std::u16string& remoteName, int64_t timestamp)
		: ChatRequest(kChatRequest_Message, timestamp), id_(id), remoteName_(remoteName)
	{
	}

	MessageRequest::~MessageRequest()
	{
	}

	int64_t MessageRequest::getMessageId()
	{
		return id_;
	}

	std::u16string MessageRequest::getRemoteName()
	{
		return remoteName_;
	}
}
