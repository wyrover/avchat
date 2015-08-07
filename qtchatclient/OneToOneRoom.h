#pragma once

#include <QDialog>
#include "ui_OneToOneRoom.h"
#include "../chatclient/ChatClient.h"
class BubbleTextObject;
class OneToOneRoom : public QDialog
{
	Q_OBJECT
public:
	OneToOneRoom(ChatClient* client, const std::wstring& remote);
	~OneToOneRoom();
	void onNewMessage(const std::wstring& remote, time_t timestamp, const std::wstring& message);
	virtual void accept();

private:
	void addMessage(const std::wstring& username, time_t timestamp, const std::wstring& message);
	Ui::UiOneToOneRoom ui;
	std::wstring remote_;
	ChatClient* client_;
	void onOk();
	void onClose();
};