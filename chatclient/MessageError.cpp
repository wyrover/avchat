#include "stdafx.h"
#include "MessageError.h"
namespace avc
{
	MessageError::MessageError(int64_t id, const std::u16string& remoteName)
		: ChatError(kChatError_Message), id_(id), remoteName_(remoteName)
	{
	}

	MessageError::~MessageError()
	{
	}

	int MessageError::getId() const
	{
		return id_;
	}

	std::u16string MessageError::getRemoteName() const
	{
		return remoteName_;
	}
}
