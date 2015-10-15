#include "stdafx.h"
#include "LoginError.h"

namespace avc
{
	LoginError::LoginError() 
		: ChatError(avc::kChatError_Login)
	{
		succeeded_ = false;
	}


	LoginError::LoginError(const std::u16string& authKey)
		: ChatError(avc::kChatError_Login)
	{
		succeeded_ = true;
		authKey_ = authKey;
	}

	LoginError::~LoginError()
	{
	}

	bool LoginError::isSucceeded() const
	{
		return succeeded_;
	}

	std::u16string LoginError::getAuthKey() const
	{
		return authKey_;
	}

}

