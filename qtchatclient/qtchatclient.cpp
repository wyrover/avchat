#include "qtchatclient.h"
#include <QStringList>
#include <ATLComTime.h>
#include "OneToOneRoom.h"
#include "BubbleTextObject.h"

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
		addMessage(sender, timestamp, message);
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

__override void qtchatclient::onFileRequest(const std::wstring& sender, int64_t timestamp, bool isFolder,
	const std::wstring& filename, int64_t fileSize)
{

}

void qtchatclient::addMessage(const QString& username, time_t timestamp, const QString& message)
{
	textBrowser->addMessage(username, username.toStdWString() == client_->getUsername(), timestamp, message);
}