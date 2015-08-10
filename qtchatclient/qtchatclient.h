#ifndef QTCHATCLIENT_H
#define QTCHATCLIENT_H

#include <QtWidgets/QMainWindow>
#include <QStringListModel>
#include <stdint.h>
#include "ui_qtchatclient.h"
#include "../chatclient/ChatClient.h"

class OneToOneRoom;
class BubbleTextObject;

//¹«¹²ÁÄÌìÊÒ
class qtchatclient : public QMainWindow, public IChatClientController, public Ui::qtchatclientClass
{
	Q_OBJECT

public:
	qtchatclient(ChatClient* client, QWidget *parent = 0);
	~qtchatclient();
	virtual __override void onNewMessage(const std::wstring& sender, const std::wstring& username, int64_t timestamp, const std::wstring& message);
	virtual __override void onNewUserList();
	virtual __override void onFileRequest(const std::wstring& sender, int64_t timestamp, bool isFolder,
		const std::wstring& filename, int64_t fileSize);

private:
	ChatClient* client_;
	QStringListModel userListModel_;
	std::map<std::wstring, OneToOneRoom*> oneMap_;
	OneToOneRoom* getRoom(const std::wstring& username);
	void addMessage(const QString& username, time_t timestamp, const QString& message);
	BubbleTextObject* fa_;

private slots:
	void onUiNewMessage(const QString& sender, const QString& username, qint64 timestamp, const QString& message);
	void onSendClicked();
	void onUsernameDoubleClicked(const QModelIndex& index);
	void onAddPicClicked();

signals:
	void uiNewMessage(const QString& sender, const QString& recver, qint64 timestamp, const QString& message);
};

#endif // QTCHATCLIENT_H
