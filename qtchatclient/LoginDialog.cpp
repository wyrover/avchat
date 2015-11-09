#include "LoginDialog.h"
#include "../chatclient/ChatClient.h"
#include <QDesktopServices>
#include <QPainter>
#include <qmath.h>
#include <QStandardPaths>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "../common/StringUtils.h"
#include "../chatclient/ChatError.h"
#include "../chatclient/LoginError.h"
#include "CustomShadowEffect.h"
#include "ChatClientController.h"

LoginDialog::LoginDialog(avc::ChatClient* client)
{
	setupUi(this);
	setStyleSheet("#mainWidget{ background: url(:Resources/logindialog.png); background-repeat: no-repeat; border:none; }");
	emailEdit->setFocus();
	emailEdit->setText("jcyangzh@gmail.com");
	passwordEdit->setText("123456");
	registerButton->setCursor(Qt::PointingHandCursor);

	auto controller = dynamic_cast<ChatClientController*>(client->controller());
	controller->setLoginDialog(this);
	client_ = client;
	connect(loginButton, SIGNAL(clicked()), this, SLOT(startLogin()));
	connect(registerButton, SIGNAL(clicked()), this, SLOT(openRegisterUrl()));
	connect(this, SIGNAL(uiChatError(avc::ChatError*)),
			this, SLOT(onUiChatError(avc::ChatError*)), Qt::QueuedConnection);
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::onChatError(avc::ChatError *error)
{
	emit uiChatError(error);
}

void LoginDialog::startLogin()
{
	auto username = emailEdit->text();
	auto password = passwordEdit->text();
	if (username.isEmpty() || password.isEmpty()) {
		setError(tr("username or password could not be empty"));
		return;
	}
	if (client_->login(username.toStdU16String(), password.toStdU16String()) != H_OK) {
		setError(tr("login failed"));
		return;
	} else {
		std::string homeDir = getpwuid(getuid())->pw_dir;
        homeDir += "/.avchat";
        client_->setImageCacheDir(su::u8to16(homeDir));
	}
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
	QDialog::paintEvent(event);
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBrush(QBrush(QColor(0xEBF2F9)));
	painter.drawRect(5, 5, this->width() - 10, this->height() - 10);
}

void LoginDialog::onUiChatError(avc::ChatError *error)
{
	auto loginError = dynamic_cast<avc::LoginError*>(error);
	if (loginError && loginError->isSucceeded()) {
		auto controller = dynamic_cast<ChatClientController*>(client_->controller());
		controller->setAuthKey(loginError->getAuthKey());
		QDialog::accept();
	} else {
		setError(tr("login failed"));
	}
}
