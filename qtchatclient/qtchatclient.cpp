#include "qtchatclient.h"
#include <QStringList>
#include <ATLComTime.h>

qtchatclient::qtchatclient(ChatClient* client, QWidget *parent)
	: QMainWindow(parent), client_(client)
{
	setupUi(this);
	connect(this, SIGNAL(uiNewUserList(const std::vector<std::wstring>&)), this, SLOT(onUiNewUserList(const std::vector<std::wstring>&)));
	connect(this, SIGNAL(uiNewMessage(const std::wstring&, const std::wstring&, int64_t, const std::wstring&)),
		this, SLOT(onUiNewMessage(const std::wstring&, const std::wstring&, int64_t, const std::wstring&)));
	connect(pushButton, SIGNAL(clicked()), this, SLOT(onSendClicked()));
	userListView->setModel(&userListModel_);
	client_->setController(this);
	client_->startThread();
}

qtchatclient::~qtchatclient()
{
}

void qtchatclient::onNewMessage(const std::wstring& sender, const std::wstring& username, int64_t timestamp, const std::wstring& message)
{
	//emit uiNewMessage(sender, username, message);
	onUiNewMessage(sender, username, timestamp, message);
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

void qtchatclient::onUiNewMessage(const std::wstring& sender, const std::wstring& recver, int64_t timestamp, const std::wstring& message)
{
	COleDateTime oleTime((time_t)timestamp);
	QString timeStr = QString::fromWCharArray(oleTime.Format(VAR_TIMEVALUEONLY).GetBuffer(0));
	
	textBrowser->insertHtml(QString(R"(<font color="blue">%1 %2</font><br>)").arg(QString::fromStdWString(sender), timeStr));
	textBrowser->insertPlainText(QString::fromStdWString(message + L"\r\n"));
}

void qtchatclient::onSendClicked()
{	
	client_->sendMessage(L"all", textEdit->toPlainText().toStdWString());
}
