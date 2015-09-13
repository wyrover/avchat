#include "stdafx.h"
#include "ChatRequest.h"
namespace avc
{
	ChatRequest::ChatRequest(int type, int64_t timestamp)
		: type_(type), timestamp_(timestamp)
	{
	}


	ChatRequest::~ChatRequest()
	{
	}

	int ChatRequest::getType() const
	{
		return type_;
	}

	int64_t ChatRequest::getTimestamp() const
	{
		return timestamp_;
	}
}
