#ifndef QTCHATCLIENT_H
#define QTCHATCLIENT_H

#include <QtWidgets/QMainWindow>
#include <QStringListModel>
#include <stdint.h>
#include "ui_qtchatclient.h"
#include "../chatclient/ChatClient.h"

class OneToOneRoom;

//¹«¹²ÁÄÌìÊÒ
class qtchatclient : public QMainWindow, public IChatClientController, public Ui::qtchatclientClass
{
	Q_OBJECT

public:
	qtchatclient(ChatClient* client, QWidget *parent = 0);
	~qtchatclient();
	virtual void onNewMessage(const std::wstring& sender, const std::wstring& username, int64_t timestamp, const std::wstring& message);
	virtual void onNewUserList();

private:
	ChatClient* client_;
	QStringListModel userListModel_;
	std::map<std::wstring, OneToOneRoom*> oneMap_;
	OneToOneRoom* getRoom(const std::wstring& username);

private slots:
	void onUiNewMessage(const QString& sender, const QString& username, qint64 timestamp, const QString& message);
	void onSendClicked();
	void onUsernameDoubleClicked(const QModelIndex& index);

signals:
	void uiNewMessage(const QString& sender, const QString& recver, qint64 timestamp, const QString& message);
};

#endif // QTCHATCLIENT_H
