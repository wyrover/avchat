#pragma once

#include <QDialog>
#include "DropShadowWidget.h"
#include "ui_OneToOneRoom.h"
#include "../chatclient/ChatClient.h"

class OneToOneRoom : public DropShadowWidget, public Ui::OneToOneRoom
{
	Q_OBJECT
public:
	OneToOneRoom(ChatClient* client, const std::wstring& remote);
	~OneToOneRoom();

	void addMessage(const QString& username, time_t timestamp, const QString& message);
	void onFileRequest(const std::wstring& sender, int64_t timestamp, bool isFolder,
		const std::wstring& filename, int64_t fileSize);
	void onMessageSendError(int64_t);

protected:
	virtual __override void paintEvent(QPaintEvent *event);

private:
	std::wstring remote_;
	ChatClient* client_;

private:

private slots:
	void onSendClicked();
	void onAddPicClicked();
	void sendFile();
};