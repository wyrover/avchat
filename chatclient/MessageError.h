#pragma once

#include "ChatError.h"
namespace avc
{
	class MessageError : public ChatError
	{
	public:
		MessageError(int64_t id, const std::wstring& remoteName);
		~MessageError();
		int getId() const;
		std::wstring getRemoteName() const;
	private:
		int64_t id_;
		std::wstring remoteName_;
	};

}
