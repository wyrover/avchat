#pragma once

#include "ChatError.h"
#include <string>

namespace avc
{
	class LoginError : public ChatError
	{
		public:
			LoginError();
			LoginError(const std::u16string& authKey);
			virtual ~LoginError();
			bool isSucceeded() const;
			std::u16string getAuthKey() const;

		private:
			bool succeeded_;
			std::u16string authKey_;
	};
}
