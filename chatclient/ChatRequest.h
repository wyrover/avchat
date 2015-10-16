#pragma once
#include <stdint.h>
namespace avc
{
	enum ChatRequestType {
		kChatRequest_Message,
		kChatRequest_SendFile,
		kChatRequest_BuildP2p,
	};

	class ChatRequest
	{
	public:
		ChatRequest(int type, int64_t timestamp);
		virtual ~ChatRequest();
		int getType() const;
		int64_t getTimestamp() const;
	private:
		int type_;
		int64_t timestamp_;
	};
}
