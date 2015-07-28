#ifndef QTCHATCLIENT_H
#define QTCHATCLIENT_H

#include <QtWidgets/QMainWindow>
#include <QStringListModel>
#include "ui_qtchatclient.h"
#include "../chatclient/ChatClient.h"

Q_DECLARE_METATYPE(std::wstring)
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
private slots:
	void onUiNewUserList(const std::vector<std::wstring>& userList);
	void onUiNewMessage(const std::wstring& sender, const std::wstring& username, int64_t timestamp, const std::wstring& message);
	void onSendClicked();
signals:
	void uiNewUserList(const std::vector<std::wstring>& userList);
	void uiNewMessage(const std::wstring& sender, const std::wstring& recver, int64_t timestamp, const std::wstring& message);
	std::map<std::wstring, OneToOneRoom*> oneMap_;
};

#endif // QTCHATCLIENT_H
