#include "LoginDialog.h"
#include "../chatclient/ChatClient.h"
#include <Shlobj.h>
#include <Shlwapi.h>
#include <QDesktopServices>
#include <QPainter>
#include <qmath.h>
#include "CustomShadowEffect.h"

LoginDialog::LoginDialog(avc::ChatClient* client)
{
	setupUi(this);
	setStyleSheet("#mainWidget{ background: url(:Resources/logindialog.png); background-repeat: no-repeat; border:none; }");
	emailEdit->setFocus();
	emailEdit->setText("jcyangzh@gmail.com");
	passwordEdit->setText("123456");
	registerButton->setCursor(Qt::PointingHandCursor);

	client_ = client;
	connect(loginButton, SIGNAL(clicked()), this, SLOT(login()));
	connect(registerButton, SIGNAL(clicked()), this, SLOT(openRegisterUrl()));
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::login()
{
	auto username = emailEdit->text();
	auto password = passwordEdit->text();
	if (username.isEmpty() || password.isEmpty()) {
		setError(tr("username or password could not be empty"));
		return;
	}
	if (client_->login(username.toStdWString(), password.toStdWString()) != H_OK) {
		setError(tr("login failed"));
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

void LoginDialog::setError(const QString& str)
{
	if (str.isEmpty()) {
		errorLabel->setVisible(false);
	} else {
		errorLabel->setVisible(true);
		errorLabel->setStyleSheet("color: rgb(178, 21, 21); background-color: rgb(255, 255, 191);");
		errorLabel->setText(str);
	}
}

void LoginDialog::openRegisterUrl()
{
	QDesktopServices::openUrl(QUrl("http://avchat.fakecoder.com/register"));
}

void LoginDialog::paintEvent(QPaintEvent *event)
{
	__super::paintEvent(event);
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBrush(QBrush(QColor(0xEBF2F9)));
	painter.drawRect(5, 5, this->width() - 10, this->height() - 10);
}
