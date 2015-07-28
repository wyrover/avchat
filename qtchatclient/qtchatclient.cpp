#include "qtchatclient.h"
#include <QStringList>
#include <ATLComTime.h>
#include "OneToOneRoom.h"

qtchatclient::qtchatclient(ChatClient* client, QWidget *parent)
	: QMainWindow(parent), client_(client)
{
	setupUi(this);
	connect(this, SIGNAL(uiNewMessage(const QString&, const QString&, qint64, const QString&)),
		this, SLOT(onUiNewMessage(const QString&, const QString&, qint64, const QString&)), Qt::QueuedConnection);
	connect(pushButton, SIGNAL(clicked()), this, SLOT(onSendClicked()));
	userListView->setModel(&userListModel_);
	userListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(userListView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onUsernameDoubleClicked(const QModelIndex&)));
	client_->setController(this);
	client_->startThread();
	setWindowTitle(QString("%1@public chat room").arg(QString::fromStdWString(client_->getUsername())));
}

qtchatclient::~qtchatclient()
{
}

void qtchatclient::onNewMessage(const std::wstring& sender, const std::wstring& username, int64_t timestamp, const std::wstring& message)
{
	emit uiNewMessage(QString::fromStdWString(sender), QString::fromStdWString(username), timestamp, QString::fromStdWString(message));
}

void qtchatclient::onNewUserList()
{
	auto userlist = client_->getUserList();
	QStringList list;
	for (auto name : userlist) {
		list << QString::fromStdWString(name);
	}
	userListModel_.setStringList(list);
}

void qtchatclient::onUiNewMessage(const QString& sender, const QString& recver, qint64 timestamp, const QString& message)
{
	if (recver == "all") {
		COleDateTime oleTime((time_t)timestamp);
		QString timeStr = QString::fromWCharArray(oleTime.Format(VAR_TIMEVALUEONLY).GetBuffer(0));
		textBrowser->insertHtml(QString(R"(<font color="blue">%1 %2</font><br>)").arg(sender, timeStr));
		textBrowser->insertPlainText(message + "\r\n");
	} else {
		auto room = getRoom(sender.toStdWString());
		room->show();
		room->onNewMessage(sender.toStdWString(), timestamp, message.toStdWString());
	}
}

void qtchatclient::onSendClicked()
{	
	client_->sendMessage(L"all", textEdit->toPlainText().toStdWString(), time(NULL));
	textEdit->clear();
}

void qtchatclient::onUsernameDoubleClicked(const QModelIndex& index)
{
	auto data = userListModel_.data(index, Qt::DisplayRole).toString();
	auto room = getRoom(data.toStdWString());
	room->show();
}	

OneToOneRoom* qtchatclient::getRoom(const std::wstring& username)
{
	if (!oneMap_.count(username)) {
		auto oneRoom = new OneToOneRoom(client_,username);
		oneMap_[username] = oneRoom;
	}
	return oneMap_[username];
}
