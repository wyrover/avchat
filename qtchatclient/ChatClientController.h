#pragma once
#include "../chatclient/IChatClientController.h"

class LoginDialog;

class ChatClientController : public avc::IChatClientController
{
public:
        ChatClientController();
        void setMainController(avc::IChatClientController* controller);
        void setLoginDialog(LoginDialog* loginDialog);
        virtual  void onNewMessage(const std::u16string& sender, const std::u16string& username,
                                   int64_t timestamp, const std::u16string& message);
        virtual  void onNewUserList();
        virtual  void onFileRequest(const std::u16string& sender, int64_t timestamp, bool isFolder,
                                    const std::u16string& filename, int64_t fileSize);
        virtual  void onFileRequestAck(const std::u16string& sender, int64_t timestamp, bool allow);
        virtual  void onChatError(avc::ChatError* error);
        void setAuthKey(const std::u16string& key);
        std::u16string getAuthKey() const;

private:
        avc::IChatClientController* mainController_;
        std::u16string authKey_;
        LoginDialog* loginDialog_;
};
