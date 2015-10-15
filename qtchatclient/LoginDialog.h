#pragma once

#include <QDialog>
#include <QMouseEvent>
#include "ui_logindialog.h"
#include "DropShadowWidget.h"
class ChatClientController;
namespace avc
{
    class ChatClient;
    class ChatError;
}

class LoginDialog : public DropShadowWidget, public Ui::LoginDialog
{
	Q_OBJECT
public:
    LoginDialog(avc::ChatClient* client);
    ~LoginDialog();
    void onChatError(avc::ChatError* error);

protected:
    virtual void paintEvent(QPaintEvent *event);

private slots:
    void onUiChatError(avc::ChatError* error);
    void startLogin();
    void openRegisterUrl();

signals:
    void uiChatError(avc::ChatError* error);

private:
    void setError(const QString& str);
	avc::ChatClient* client_;
};
