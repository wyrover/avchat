#include "ChatClientController.h"

ChatClientController::ChatClientController()
{
}

void ChatClientController::onNewMessage(const std::u16string &sender, const std::u16string &username, int64_t timestamp, const std::u16string &message)
{
}

void ChatClientController::onNewUserList()
{
}

void ChatClientController::onFileRequest(const std::u16string &sender, int64_t timestamp, bool isFolder, const std::u16string &filename, int64_t fileSize)
{
}

void ChatClientController::onFileRequestAck(const std::u16string &sender, int64_t timestamp, bool allow)
{
}

void ChatClientController::onChatError(avc::ChatError *error)
{
}

void ChatClientController::setAuthKey(const std::u16string &key)
{
        authKey_ = key;
}

std::u16string ChatClientController::getAuthKey() const
{
        return authKey_;
}

