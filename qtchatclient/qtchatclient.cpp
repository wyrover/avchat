#include "qtchatclient.h"
#include <QStringList>
#include <QTextStream>
#include <QScrollBar>
#include <QFileDialog>
#include <QDebug>
#include <QMenu>
#include <QGraphicsDropShadowEffect>
#include <QSizeGrip>
#include <qshortcut.h>
#include "TitleBar.h"
#include "OneToOneRoom.h"
#include "CustomShadowEffect.h"
#include "Utils.h"
#include "ChatClientController.h"
#include "../chatclient/MessageError.h"

qtchatclient::qtchatclient(avc::ChatClient* client, QWidget *parent)
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
	titleBar->setTitle(QString("%1@public chat room").arg(QString::fromStdU16String(client_->getEmail())));
	userListView->setModel(&userListModel_);
	textEdit->setFocus();

	auto shortcut = new QShortcut(QKeySequence(Qt::CTRL, Qt::Key_T), this);
	shortcut->setAutoRepeat(false);
    //connect(shortcut, SIGNAL(activated()), this, SLOT(keke()));
    //connect(shortcut, SIGNAL(activatedAmbiguously()), this, SLOT(onSendClicked()));

	connect(userListView, SIGNAL(doubleClicked(const QModelIndex&)),
		this, SLOT(onUsernameDoubleClicked(const QModelIndex&)));
	connect(this, SIGNAL(uiNewMessage(const QString&, const QString&, qint64, const QString&)),
		this, SLOT(onUiNewMessage(const QString&, const QString&, qint64, const QString&)), Qt::QueuedConnection);
    connect(this, SIGNAL(uiChatError(avc::ChatError*)),
        this, SLOT(onUiChatError(avc::ChatError*)), Qt::QueuedConnection);
	connect(this, SIGNAL(uiFileTransferRequest(const QString&, qint64, bool, const QString& , int)),
		this, SLOT(onUiFileTransferRequest(const QString&, qint64, bool, const QString& , int)));
	connect(pushButton, SIGNAL(clicked()), this, SLOT(onSendClicked()));
	connect(addPicBtn, SIGNAL(clicked()), this, SLOT(onAddPicClicked()));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(onCloseButtonClicked()));

    auto controller =  dynamic_cast<ChatClientController*>(client_->controller());
    controller->setMainController(this);
    onNewUserList();
	//createTray();
}

qtchatclient::~qtchatclient()
{
}

void qtchatclient::onNewMessage(const std::u16string& sender, const std::u16string& username,
	int64_t timestamp, const std::u16string& message)
{
	emit uiNewMessage(QString::fromStdU16String(sender), QString::fromStdU16String(username),
		timestamp, QString::fromStdU16String(message));
}

 void qtchatclient::onChatError(avc::ChatError* error)
{
    emit uiChatError(error);
}

void qtchatclient::onNewUserList()
{
	auto userlist = client_->getUserList();
	QStringList list;
	for (auto name : userlist) {
		list << QString::fromStdU16String(name);
	}
	userListModel_.setStringList(list);
}

void qtchatclient::onUiNewMessage(const QString& sender, const QString& recver, qint64 timestamp, const QString& message)
{
	if (sender == QString::fromStdU16String(client_->getEmail()))
		return;

	if (recver == "all") {
		addMessage(sender, timestamp, message);
	} else {
		auto room = getRoom(sender.toStdU16String());
		room->show();
		room->addMessage(sender, timestamp, message);
	}
}

void qtchatclient::onSendClicked()
{	
	auto html = textEdit->toHtml();
	auto text = textEdit->toPlainText().toStdU16String();
	auto message = Utils::textEditToMessageText(textEdit);
	auto timestamp = time(NULL);
    client_->sendMessage(u"all", message, timestamp);
	addMessage(QString::fromStdU16String(client_->getEmail()), timestamp, QString::fromStdU16String(message));
	textEdit->clear();
}

void qtchatclient::onUsernameDoubleClicked(const QModelIndex& index)
{
	auto data = userListModel_.data(index, Qt::DisplayRole).toString();
	auto room = getRoom(data.toStdU16String());
	room->show();
}	

OneToOneRoom* qtchatclient::getRoom(const std::u16string& username)
{
	if (!oneMap_.count(username)) {
		auto oneRoom = new OneToOneRoom(client_,username);
		oneMap_[username] = oneRoom;
	}
	return oneMap_[username];
}

 void qtchatclient::onFileRequest(const std::u16string& sender, int64_t timestamp, bool isFolder,
	const std::u16string& filename, int64_t fileSize)
{
	emit uiFileTransferRequest(QString::fromStdU16String(sender), timestamp, isFolder, QString::fromStdU16String(filename), (int)fileSize);
}

void qtchatclient::addMessage(const QString& username, time_t timestamp, const QString& message)
{
	textBrowser->addMessage(username, username.toStdU16String() == client_->getEmail(), timestamp,
		message, QString::fromStdU16String(client_->getImageDir()));
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

 void qtchatclient::closeEvent(QCloseEvent* event)
{
	//if (trayIcon_->isVisible()) {
	//	hide();
	//	event->ignore();
	//}
	event->accept();
}

void qtchatclient::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);
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

void qtchatclient::onUiChatError(avc::ChatError* error)
{
        auto type = error->getType();
        switch (type) {
        case avc::kChatError_Message: {
                auto msgError = dynamic_cast<avc::MessageError*>(error);
                auto name = msgError->getRemoteName();
                if (name == u"all")
                        textBrowser->markSendError(msgError->getId());
                else {
                        auto room = getRoom(name);
                }
        }
                break;
        default:
                break;
        }
        delete error;
}

void qtchatclient::onCloseButtonClicked()
{
	textBrowser->markSendError(0);
}

void qtchatclient::onUiFileTransferRequest(const QString& sender, qint64 timestamp,
	bool isFolder, const QString& filename, int fileSize)
{
	auto room = getRoom(sender.toStdU16String());
	room->addFileRequest(sender, timestamp, false, filename, fileSize);
	room->show();
}

 void qtchatclient::onFileRequestAck(const std::u16string& sender, int64_t timestamp, bool allow)
{

}
