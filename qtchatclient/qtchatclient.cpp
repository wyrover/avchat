#include "qtchatclient.h"
#include <QStringList>
#include <ATLComTime.h>
#include <QTextStream>
#include <QScrollBar>
#include <QShortCut>
#include <QFileDialog>
#include <QDebug>
#include <QMenu>
#include <QGraphicsDropShadowEffect>
#include <QSizeGrip>
#include "TitleBar.h"
#include "OneToOneRoom.h"
#include "CustomShadowEffect.h"
#include "Utils.h"

qtchatclient::qtchatclient(ChatClient* client, QWidget *parent)
	: DropShadowWidget(parent), client_(client)
{
	setupUi(this);
	rightLayout->addWidget(new QSizeGrip(this), 0, Qt::AlignBottom | Qt::AlignRight);
	userListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	userListView->setFrameStyle(QFrame::NoFrame);
	textBrowser->setFrameStyle(QFrame::NoFrame);
	textEdit->setFrameStyle(QFrame::NoFrame);

	QFile file(":/Resources/scrollbar.qss");
	file.open(QFile::ReadOnly | QFile::Text);
	QTextStream in(&file);
	QString content = in.readAll();
	textBrowser->verticalScrollBar()->setStyleSheet(content);
	textEdit->verticalScrollBar()->setStyleSheet(content);
	userListView->verticalScrollBar()->setStyleSheet(content);

	addPicBtn->setIcon(QIcon(":/Resources/image.png"));
	addVoiceBtn->setIcon(QIcon(":/Resources/voice.png"));
	titleBar->setTitle(QString("%1@public chat room").arg(QString::fromStdWString(client_->getEmail())));
	userListView->setModel(&userListModel_);
	textEdit->setFocus();

	auto shortcut = new QShortcut(QKeySequence(Qt::CTRL, Qt::Key_T), this);
	shortcut->setAutoRepeat(false);
	connect(shortcut, SIGNAL(activated()), this, SLOT(keke()));
	//connect(shortcut, SIGNAL(activatedAmbiguously()), this, SLOT(onSendClicked()));

	connect(userListView, SIGNAL(doubleClicked(const QModelIndex&)),
		this, SLOT(onUsernameDoubleClicked(const QModelIndex&)));
	connect(this, SIGNAL(uiNewMessage(const QString&, const QString&, qint64, const QString&)),
		this, SLOT(onUiNewMessage(const QString&, const QString&, qint64, const QString&)), Qt::QueuedConnection);
	connect(this, SIGNAL(uiMessageSendError(qint64)),
		this, SLOT(onUiMessageSendError(qint64)), Qt::QueuedConnection);
	connect(pushButton, SIGNAL(clicked()), this, SLOT(onSendClicked()));
	connect(addPicBtn, SIGNAL(clicked()), this, SLOT(onAddPicClicked()));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(onCloseButtonClicked()));

	//createTray();
	client_->setController(this);
	client_->start();
}

qtchatclient::~qtchatclient()
{
}

void qtchatclient::onNewMessage(const std::wstring& sender, const std::wstring& username,
	int64_t timestamp, const std::wstring& message)
{
	emit uiNewMessage(QString::fromStdWString(sender), QString::fromStdWString(username),
		timestamp, QString::fromStdWString(message));
}

__override void qtchatclient::onMessageSendError(int64_t timestamp)
{
	emit uiMessageSendError(timestamp);
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
	if (sender == QString::fromStdWString(client_->getEmail()))
		return;

	if (recver == "all") {
		addMessage(sender, timestamp, message);
	} else {
		auto room = getRoom(sender.toStdWString());
		room->show();
		room->addMessage(sender, timestamp, message);
	}
}

void qtchatclient::onSendClicked()
{	
	auto html = textEdit->toHtml();
	auto text = textEdit->toPlainText().toStdWString();
	auto message = Utils::textEditToMessageText(textEdit);
	auto timestamp = time(NULL);
	client_->sendMessage(L"all", message, timestamp);
	addMessage(QString::fromStdWString(client_->getEmail()), timestamp, QString::fromStdWString(message));
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
	textBrowser->addMessage(username, username.toStdWString() == client_->getEmail(), timestamp,
		message, QString::fromStdWString(client_->getImageDir()));
}

void qtchatclient::onAddPicClicked()
{
	auto fileName = QFileDialog::getOpenFileName(this,
		tr("Open Image File"), "", tr("Image Files (*.jpg;*.png;*.gif;*.jpeg)"));
	Utils::addImageToTextEdit(fileName, textEdit);
}

void qtchatclient::createActions()
{
	loginAction_ = new QAction(tr("online"), this);
	connect(loginAction_, SIGNAL(triggered()), this, SLOT(login()));

	logoutAction_ = new QAction(tr("offline"), this);
	connect(logoutAction_, SIGNAL(triggered()), this, SLOT(logout()));

	restoreAction_ = new QAction(tr("restore"), this);
	connect(restoreAction_, SIGNAL(triggered()), this, SLOT(showNormal()));

	quitAction_ = new QAction(tr("quit"), this);
	connect(quitAction_, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void qtchatclient::createTrayIcon()
{
	trayIconMenu_ = new QMenu(this);
	trayIconMenu_->addAction(loginAction_);
	trayIconMenu_->addAction(logoutAction_);
	trayIconMenu_->addAction(restoreAction_);
	trayIconMenu_->addSeparator();
	trayIconMenu_->addAction(quitAction_);

	trayIcon_ = new QSystemTrayIcon(this);
	trayIcon_->setContextMenu(trayIconMenu_);
}

__override void qtchatclient::closeEvent(QCloseEvent* event)
{
	//if (trayIcon_->isVisible()) {
	//	hide();
	//	event->ignore();
	//}
	event->accept();
}

void qtchatclient::paintEvent(QPaintEvent *event)
{
	__super::paintEvent(event);
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBrush(QBrush(QColor(0xEBF2F9)));
	painter.drawRect(5, 5, this->width() - 10, this->height() - 10);
}

void qtchatclient::createTray()
{
	createActions();
	createTrayIcon();
	trayIcon_->setIcon(QIcon(":/Resources/chat.ico"));
	trayIcon_->show();
}

void qtchatclient::onUiMessageSendError(qint64 timestamp)
{
	textBrowser->markError(timestamp);
}

void qtchatclient::onCloseButtonClicked()
{
	textBrowser->markError(0);
}
