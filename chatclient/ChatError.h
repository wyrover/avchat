#pragma once

namespace avc
{
	enum ChatErrorType {
		kChatError_Message,
	};

	class ChatError
	{
	public:
		ChatError(int type);
		virtual ~ChatError();
		int getType() const;
	private:
		int type_;
	};
}
