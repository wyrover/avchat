#include "ChatClientController.h"
#include "LoginDialog.h"

ChatClientController::ChatClientController()
{
    mainController_ = nullptr;
    loginDialog_ = nullptr;
}

void ChatClientController::setMainController(avc::IChatClientController *controller)
{
        mainController_ = controller;
}

void ChatClientController::setLoginDialog(LoginDialog *loginDialog)
{
        loginDialog_ = loginDialog;
}

void ChatClientController::onNewMessage(const std::u16string &sender, const std::u16string &username, int64_t timestamp, const std::u16string &message)
{
        if (mainController_) {
                return mainController_->onNewMessage(sender, username, timestamp, message);
        }
}

void ChatClientController::onNewUserList()
{
        if (mainController_) {
                return mainController_->onNewUserList();
        }
}

void ChatClientController::onFileRequest(const std::u16string &sender, int64_t timestamp, bool isFolder, const std::u16string &filename, int64_t fileSize)
{
        if (mainController_)  {
                return mainController_->onFileRequest(sender, timestamp, isFolder, filename, fileSize);
        }
}

void ChatClientController::onFileRequestAck(const std::u16string &sender, int64_t timestamp, bool allow)
{
        if (mainController_) {
                return mainController_->onFileRequestAck(sender, timestamp, allow);
        }
}

void ChatClientController::onChatError(avc::ChatError *error)
{
        if (loginDialog_) {
                return loginDialog_->onChatError(error);
        }

        if (mainController_) {
                return mainController_->onChatError(error);
        }
}

void ChatClientController::setAuthKey(const std::u16string &key)
{
        authKey_ = key;
}

std::u16string ChatClientController::getAuthKey() const
{
        return authKey_;
}

