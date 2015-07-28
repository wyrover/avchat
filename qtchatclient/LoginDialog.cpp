#include "LoginDialog.h"
#include "../chatclient/ChatClient.h"

LoginDialog::LoginDialog(ChatClient* client)
{
	setupUi(this);
	lineEdit->setFocus();
	client_ = client;
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(onOk()));
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::accept()
{
	auto username = lineEdit->text();
	auto password = lineEdit_2->text();
	if (username.isEmpty() || password.isEmpty()) {
		setError(L"用户名或者密码不能够为空");
		return;
	}
	if (!client_->login(username.toStdWString(), password.toStdWString())) {
		setError(L"登录失败");
		return;
	}
	__super::accept();
}

void LoginDialog::setError(const std::wstring& str)
{
	label_3->setText(QString::fromStdWString(str));
}