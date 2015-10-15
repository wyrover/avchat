#include "OneToOneRoom.h"
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
#include "Utils.h"
#include "../chatclient/RequestFilesInfo.h"
#include "../common/trace.h"

OneToOneRoom::OneToOneRoom(avc::ChatClient* client, const std::u16string& remote)
{
	remote_ = remote;
	client_ = client;
	setupUi(this);
	userInfoWidget->show();
	rightLayout->addWidget(new QSizeGrip(this), 0, Qt::AlignBottom | Qt::AlignRight);
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
	sendFileBtn->setIcon(QIcon(":/Resources/file.png"));
    titleBar->setTitle(QString("%1@public chat room").arg(QString::fromStdU16String(client_->getEmail())));
	textEdit->setFocus();
    titleBar->setTitle(QString("%1->%2").arg(QString::fromStdU16String(client_->getEmail()), QString::fromStdU16String(remote_)));


	new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_T), this, SLOT(test()));
	connect(pushButton, SIGNAL(clicked()), this, SLOT(onSendClicked()));
	connect(addPicBtn, SIGNAL(clicked()), this, SLOT(onAddPicClicked()));
	connect(sendFileBtn, SIGNAL(clicked()), this, SLOT(sendFile()));
	//connect(stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(stackedWidgetCurrentChanged(int)));
}

OneToOneRoom::~OneToOneRoom()
{
}

void OneToOneRoom::addMessage(const QString& username, time_t timestamp, const QString& message)
{
    textBrowser->addMessage(username, username.toStdU16String() == client_->getEmail(), timestamp,
        message, QString::fromStdU16String(client_->getImageDir()));
}

void OneToOneRoom::addFileRequest(const QString& sender, int64_t timestamp,
	bool isFolder, const QString& filename, int fileSize)
{
	fileRequestWidget->show();
	fileRequestWidget->addRecvRequest(filename, fileSize, timestamp);
	stackedWidget->setCurrentWidget(fileRequestWidget);
}

void OneToOneRoom::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBrush(QBrush(QColor(0xEBF2F9)));
	painter.drawRect(5, 5, this->width() - 10, this->height() - 10);
}

void OneToOneRoom::onSendClicked()
{
	//userInfoWidget->show();
	//stackedWidget->setCurrentWidget(userInfoWidget);
	auto html = textEdit->toHtml();
    auto text = textEdit->toPlainText().toStdU16String();
	auto message = Utils::textEditToMessageText(textEdit);
	auto timestamp = time(NULL);
	client_->sendMessage(remote_, message, timestamp);
    addMessage(QString::fromStdU16String(client_->getEmail()), timestamp, QString::fromStdU16String(message));
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
	auto filePath = QFileDialog::getOpenFileName(this,
		tr("Open file to send "), "", tr("any files (*.*)"));
	QFileInfo f(filePath);
	if (!f.exists())
		return;
	auto timestamp = time(NULL);
	fileRequestWidget->show();
	fileRequestWidget->addSendRequest(filePath, timestamp);
	stackedWidget->setCurrentWidget(fileRequestWidget);

	auto size = f.size();
	avc::RequestFilesInfo fi;
    fi.fileName = filePath.toStdU16String();
	fi.fileSize = size;
	client_->sendFileTransferRequest(remote_, fi, timestamp);
}

void OneToOneRoom::stackedWidgetCurrentChanged(int index)
{
	auto widget = stackedWidget->currentWidget();
	QRect rc;
	rc.setBottomRight(stackedWidget->geometry().bottomRight());
	rc.setTopLeft(QPoint(rc.bottom() - widget->height(), rc.right() - widget->width()));
	stackedWidget->setGeometry(rc);
	updateGeometry();
}

void OneToOneRoom::test()
{
	TRACE("stackedwi size (%d, %d)\n", stackedWidget->sizeHint().width(), stackedWidget->sizeHint().height()); 
	TRACE("filerequest size (%d, %d)\n", fileRequestWidget->sizeHint().width(), fileRequestWidget->sizeHint().height()); 
	TRACE("this size (%d, %d)\n", sizeHint().width(), sizeHint().height()); 
}

void OneToOneRoom::markSendError(int64_t id)
{
	textBrowser->markSendError(id);
}
