#pragma once

#include <QDialog>
#include "DropShadowWidget.h"
#include "ui_OneToOneRoom.h"
#include "../chatclient/ChatClient.h"

class OneToOneRoom : public DropShadowWidget, public Ui::OneToOneRoom
{
	Q_OBJECT
public:
	OneToOneRoom(avc::ChatClient* client, const std::wstring& remote);
	~OneToOneRoom();

	void addMessage(const QString& username, time_t timestamp, const QString& message);
	void addFileRequest(const QString& sender, int64_t timestamp, bool isFolder,
		const QString& filename, int fileSize);
	void markSendError(int64_t id);

protected:
	virtual __override void paintEvent(QPaintEvent *event);

private:
	std::wstring remote_;
	avc::ChatClient* client_;

private:

private slots:
	void onSendClicked();
	void onAddPicClicked();
	void sendFile();
	void test();
	void stackedWidgetCurrentChanged(int index);
};