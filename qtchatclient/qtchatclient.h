#ifndef QTCHATCLIENT_H
#define QTCHATCLIENT_H

#include <QtWidgets/QMainWindow>
#include <QStringListModel>
#include <QSystemTrayIcon>
#include <QMenu>
#include <stdint.h>
#include "ui_qtchatclient.h"
#include "../chatclient/ChatClient.h"
#include "DropShadowWidget.h"

class OneToOneRoom;

//����������
class qtchatclient : public DropShadowWidget, public IChatClientController, public Ui::qtchatclientClass
{
	Q_OBJECT

public:
	qtchatclient(ChatClient* client, QWidget *parent = 0);
	~qtchatclient();
	virtual __override void onNewMessage(const std::wstring& sender, const std::wstring& username, int64_t timestamp, const std::wstring& message);
	virtual __override void onNewUserList();
	virtual __override void onFileRequest(const std::wstring& sender, int64_t timestamp, bool isFolder,
		const std::wstring& filename, int64_t fileSize);
	virtual __override void onMessageSendError(int64_t);

protected:
	virtual __override void closeEvent(QCloseEvent* event);
	virtual __override void paintEvent(QPaintEvent *event);

private:
	ChatClient* client_;
	QStringListModel userListModel_;
	std::map<std::wstring, OneToOneRoom*> oneMap_;

	QAction *loginAction_;
	QAction *logoutAction_;
	QAction *quitAction_;
	QAction *restoreAction_;
	QSystemTrayIcon *trayIcon_;
	QMenu *trayIconMenu_;
private:
	OneToOneRoom* getRoom(const std::wstring& username);
	void addMessage(const QString& username, time_t timestamp, const QString& message);
	void createTray();

private slots:
	void onUiNewMessage(const QString& sender, const QString& username, qint64 timestamp, const QString& message);
	void onUiMessageSendError(qint64 timestamp);
	void onSendClicked();
	void onCloseButtonClicked();
	void onUsernameDoubleClicked(const QModelIndex& index);
	void onAddPicClicked();
	void createActions();
	void createTrayIcon();

signals:
	void uiNewMessage(const QString& sender, const QString& recver, qint64 timestamp, const QString& message);
	void uiMessageSendError(qint64 timestamp);
};

#endif // QTCHATCLIENT_H