#ifndef QTCHATCLIENT_H
#define QTCHATCLIENT_H

#include <QtWidgets/QMainWindow>
#include <QStringListModel>
#include <QSystemTrayIcon>
#include <QMenu>
#include <stdint.h>
#include "DropShadowWidget.h"
#include "ui_qtchatclient.h"
#include "../chatclient/ChatClient.h"
#include "../chatclient/ChatError.h"

class OneToOneRoom;
class qtchatclient : public DropShadowWidget, public avc::IChatClientController, public Ui::qtchatclientClass
{
	Q_OBJECT

public:
	qtchatclient(avc::ChatClient* client, QWidget *parent = 0);
	~qtchatclient();
    virtual  void onNewMessage(const std::u16string& sender, const std::u16string& username,
                                         int64_t timestamp, const std::u16string& message);
    virtual  void onNewUserList();
    virtual  void onFileRequest(const std::u16string& sender, int64_t timestamp, bool isFolder,
		const std::u16string& filename, int64_t fileSize);
    virtual  void onFileRequestAck(const std::u16string& sender, int64_t timestamp, bool allow);
    virtual  void onChatError(avc::ChatError* error);

protected:
    virtual  void closeEvent(QCloseEvent* event);
    virtual  void paintEvent(QPaintEvent *event);

private:
	avc::ChatClient* client_;
	QStringListModel userListModel_;
	std::map<std::u16string, OneToOneRoom*> oneMap_;

	QAction *loginAction_;
	QAction *logoutAction_;
	QAction *quitAction_;
	QAction *restoreAction_;
	QSystemTrayIcon *trayIcon_;
	QMenu *trayIconMenu_;
private:
	OneToOneRoom* getRoom(const std::u16string& username);
	void addMessage(const QString& username, time_t timestamp, const QString& message);
	void createTray();

private slots:
	void onUiNewMessage(const QString& sender, const QString& username, qint64 timestamp, const QString& message);
    void onUiChatError(avc::ChatError* error);
	void onUiFileTransferRequest(const QString& sender, qint64 timestamp, bool isFolder,
		const QString& filename, int fileSize);
	void onSendClicked();
	void onCloseButtonClicked();
	void onUsernameDoubleClicked(const QModelIndex& index);
	void onAddPicClicked();
	void createActions();
	void createTrayIcon();

signals:
    void uiChatError(avc::ChatError* error);
    void uiNewMessage(const QString& sender, const QString& recver, qint64 timestamp, const QString& message);
	void uiFileTransferRequest(const QString& sender, qint64 timestamp, bool isFolder,
		const QString& filename, int fileSize);
};

#endif // QTCHATCLIENT_H
