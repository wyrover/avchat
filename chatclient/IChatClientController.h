#pragma once

#include <stdint.h>
#include <string>

namespace avc
{
	class ChatError;

	class IChatClientController {
	public:
		virtual void onNewMessage(const std::u16string& sender, const std::u16string& recver,
			int64_t timestamp, const std::u16string& message) = 0;
		virtual void onNewUserList() = 0;
		virtual void onFileRequest(const std::u16string& sender, int64_t timestamp, bool isFolder,
			const std::u16string& filename, int64_t fileSize) = 0;
		virtual void onFileRequestAck(const std::u16string& sender, int64_t timestamp, bool allow) = 0;
		virtual void onChatError(ChatError* error) = 0;
		virtual ~IChatClientController() = default;
	};
}
