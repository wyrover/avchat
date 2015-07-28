#include "qtchatclient.h"
#include <QStringList>

qtchatclient::qtchatclient(ChatClient* client, QWidget *parent)
	: QMainWindow(parent), client_(client)
{
	setupUi(this);
	connect(this, SIGNAL(uiNewUserList(const std::vector<std::wstring>&)), this, SLOT(onUiNewUserList(const std::vector<std::wstring>&)));
	connect(this, SIGNAL(uiNewMessage(const std::wstring&, const std::wstring&, const std::wstring&)),
		this, SLOT(onUiNewMessage(const std::wstring&, const std::wstring&, const std::wstring&)));
	connect(pushButton, SIGNAL(clicked()), this, SLOT(onSendClicked()));
	userListView->setModel(&userListModel_);
	client_->setController(this);
	client_->startThread();
}

qtchatclient::~qtchatclient()
{
}

void qtchatclient::onNewMessage(const std::wstring& sender, const std::wstring& username, const std::wstring& message)
{
	//emit uiNewMessage(sender, username, message);
	onUiNewMessage(sender, username, message);
}

void qtchatclient::onNewUserList()
{
	auto userlist = client_->getUserList();
	onUiNewUserList(userlist);
	//emit uiNewUserList(userlist);
}

void qtchatclient::onUiNewUserList(const std::vector<std::wstring>& userList)
{
	QStringList list;
	for (auto name : userList) {
		list << QString::fromStdWString(name);
	}
	userListModel_.setStringList(list);
}

void qtchatclient::onUiNewMessage(const std::wstring& sender, const std::wstring& username, const std::wstring& message)
{
	QString str = QString::fromStdWString(sender + L"µ½" + username + L":" + message);
	textBrowser->append(str);
}

void qtchatclient::onSendClicked()
{	
	client_->sendMessage(L"all", textEdit->toPlainText().toStdWString());
}
