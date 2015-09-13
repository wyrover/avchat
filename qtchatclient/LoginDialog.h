#pragma once

#include <QDialog>
#include <QMouseEvent>
#include "ui_logindialog.h"
#include "dropshadowwidget.h"
namespace avc
{
	class ChatClient;
}

class LoginDialog : public DropShadowWidget, public Ui::LoginDialog
{
	Q_OBJECT
public:
	LoginDialog(avc::ChatClient* client);
	~LoginDialog();

protected:
	virtual void paintEvent(QPaintEvent *event);
	private slots:
	void login();
	void openRegisterUrl();
private:
	void setError(const QString& str);
	avc::ChatClient* client_;
};