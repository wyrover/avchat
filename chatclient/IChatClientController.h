#pragma once

#include <stdint.h>
#include <string>

namespace avc
{
	class ChatError;

	class IChatClientController {
	public:
		virtual void onNewMessage(const std::wstring& sender, const std::wstring& recver,
			int64_t timestamp, const std::wstring& message) = 0;
		virtual void onNewUserList() = 0;
		virtual void onFileRequest(const std::wstring& sender, int64_t timestamp, bool isFolder,
			const std::wstring& filename, int64_t fileSize) = 0;
		virtual void onFileRequestAck(const std::wstring& sender, int64_t timestamp, bool allow) = 0;
		virtual void onChatError(ChatError* error) = 0;
	};
}
