#pragma once

#include "ChatError.h"
#include <stdint.h>
#include <string>
namespace avc
{
	class MessageError : public ChatError
	{
	public:
		MessageError(int64_t id, const std::u16string& remoteName);
		~MessageError();
		int getId() const;
		std::u16string getRemoteName() const;
	private:
		int64_t id_;
		std::u16string remoteName_;
	};

}
