#pragma once

#include <QDialog>
#include <QMouseEvent>
#include "ui_logindialog.h"
#include "dropshadowwidget.h"
class ChatClient;

class LoginDialog : public DropShadowWidget, public Ui::LoginDialog
{
	Q_OBJECT
public:
	LoginDialog(ChatClient* client);
	~LoginDialog();

protected:
	virtual void paintEvent(QPaintEvent *event);
	private slots:
	void login();
	void openRegisterUrl();
private:
	void setError(const QString& str);
	ChatClient* client_;
};