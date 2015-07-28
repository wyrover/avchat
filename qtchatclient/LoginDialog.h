#pragma once

#include <QDialog>
#include "ui_logindialog.h"
class ChatClient;
class LoginDialog : public QDialog, public Ui::Dialog
{
	Q_OBJECT
public:
	LoginDialog(ChatClient* client);
	~LoginDialog();
	virtual void accept();
private:
	void setError(const std::wstring& str);
	ChatClient* client_;
};