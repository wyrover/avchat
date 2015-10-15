#pragma once

namespace avc
{
	enum ChatErrorType {
		kChatError_Message,
		kChatError_Login,
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
