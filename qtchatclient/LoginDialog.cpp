#include "LoginDialog.h"
#include "../chatclient/ChatClient.h"

LoginDialog::LoginDialog(ChatClient* client)
{
	setupUi(this);
	lineEdit->setFocus();
	client_ = client;
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(onOk()));
	lineEdit->setText("jcyangzh@gmail.com");
	lineEdit_2->setText("123456");
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::accept()
{
	auto username = lineEdit->text();
	auto password = lineEdit_2->text();
	if (username.isEmpty() || password.isEmpty()) {
		setError(L"�û����������벻�ܹ�Ϊ��");
		return;
	}
	if (!client_->login(username.toStdWString(), password.toStdWString())) {
		setError(L"��¼ʧ��");
		return;
	}
	__super::accept();
}

void LoginDialog::setError(const std::wstring& str)
{
	label_3->setText(QString::fromStdWString(str));
}