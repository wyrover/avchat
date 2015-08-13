#include "LoginDialog.h"
#include "../chatclient/ChatClient.h"
#include <Shlobj.h>
#include <Shlwapi.h>
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
		setError(L"用户名或者密码不能够为空");
		return;
	}
	if (client_->login(username.toStdWString(), password.toStdWString()) != H_OK) {
		setError(L"登录失败");
		return;
	} else {
		TCHAR szPath[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath))) {
			PathAppend(szPath, L"\\fakecoder\\image_cache\\");
			SHCreateDirectoryEx(NULL, szPath, NULL);
			client_->setImageCacheDir(szPath);
		}
	}
	__super::accept();
}

void LoginDialog::setError(const std::wstring& str)
{
	label_3->setText(QString::fromStdWString(str));
}