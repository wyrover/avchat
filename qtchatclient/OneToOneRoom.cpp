#include "OneToOneRoom.h"
#include "BubbleTextObject.h"
#include <assert.h>
#include <ATLComTime.h>

OneToOneRoom::OneToOneRoom(ChatClient* client, const std::wstring& remote)
{
	ui.setupUi(this);
	remote_ = remote;
	client_ = client;
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(onClose()));
	setWindowTitle(QString("%1->%2").arg(QString::fromStdWString(client_->getUsername()), QString::fromStdWString(remote_)));
}

OneToOneRoom::~OneToOneRoom()
{
}

void OneToOneRoom::onNewMessage(const std::wstring& remote, time_t timestamp, const std::wstring& message)
{
	assert(remote == remote_);
	addMessage(remote, timestamp, message);
}

void OneToOneRoom::onClose()
{
}

void OneToOneRoom::accept()
{
	auto message = ui.textEdit->toPlainText().toStdWString();
	auto currTime = time(NULL);
	client_->sendMessage(remote_, message, currTime);
	ui.textEdit->clear();
	addMessage(client_->getUsername(), currTime, message);
}
 
void OneToOneRoom::addMessage(const std::wstring& username, time_t timestamp, const std::wstring& message)
{
	COleDateTime oleTime((time_t)timestamp);
	QString timeStr = QString::fromWCharArray(oleTime.Format(VAR_TIMEVALUEONLY).GetBuffer(0));
	ui.textBrowser->insertHtml(QString(R"(<font color="blue">%1 %2</font><br>)").arg(QString::fromStdWString(username), timeStr));
	ui.textBrowser->addMessage(QString::fromStdWString(username), username == client_->getUsername(), timestamp, QString::fromStdWString(message));
}