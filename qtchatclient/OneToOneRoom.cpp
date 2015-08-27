#include "OneToOneRoom.h"
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
#include "Utils.h"

OneToOneRoom::OneToOneRoom(ChatClient* client, const std::wstring& remote)
{
	remote_ = remote;
	client_ = client;
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

	addPicBtn->setIcon(QIcon(":/Resources/image.png"));
	addVoiceBtn->setIcon(QIcon(":/Resources/voice.png"));
	titleBar->setTitle(QString("%1@public chat room").arg(QString::fromStdWString(client_->getEmail())));
	textEdit->setFocus();
	titleBar->setTitle(QString("%1->%2").arg(QString::fromStdWString(client_->getEmail()), QString::fromStdWString(remote_)));

	connect(pushButton, SIGNAL(clicked()), this, SLOT(onSendClicked()));
	connect(addPicBtn, SIGNAL(clicked()), this, SLOT(onAddPicClicked()));
	connect(sendFileBtn, SIGNAL(clicked()), this, SLOT(sendFile()));
}

OneToOneRoom::~OneToOneRoom()
{
}

void OneToOneRoom::addMessage(const QString& username, time_t timestamp, const QString& message)
{
	textBrowser->addMessage(username, username.toStdWString() == client_->getEmail(), timestamp,
		message, QString::fromStdWString(client_->getImageDir()));
}

void OneToOneRoom::onFileRequest(const std::wstring& sender, int64_t timestamp,
	bool isFolder, const std::wstring& filename, int64_t fileSize)
{

}

void OneToOneRoom::onMessageSendError(int64_t)
{

}

__override void OneToOneRoom::paintEvent(QPaintEvent *event)
{
	__super::paintEvent(event);
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBrush(QBrush(QColor(0xEBF2F9)));
	painter.drawRect(5, 5, this->width() - 10, this->height() - 10);
}

void OneToOneRoom::onSendClicked()
{
	auto html = textEdit->toHtml();
	auto text = textEdit->toPlainText().toStdWString();
	auto message = Utils::textEditToMessageText(textEdit);
	client_->sendMessage(remote_, message, time(NULL));
	textEdit->clear();
}

void OneToOneRoom::onAddPicClicked()
{
	auto fileName = QFileDialog::getOpenFileName(this,
		tr("Open Image File"), "", tr("Image Files (*.jpg;*.png;*.gif;*.jpeg)"));
	Utils::addImageToTextEdit(fileName, textEdit);
}

void OneToOneRoom::sendFile()
{

}
