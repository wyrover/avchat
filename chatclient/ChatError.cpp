#include "stdafx.h"
#include "ChatError.h"

namespace avc
{
	ChatError::ChatError(int type)
		: type_(type)
	{
	}


	ChatError::~ChatError()
	{
	}

	int ChatError::getType() const
	{
		return type_;
	}
}

