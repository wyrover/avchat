#pragma once

#include <QDialog>
#include "ui_OneToOneRoom.h"

class OneToOneRoom : public QDialog
{
	Q_OBJECT
public:
	OneToOneRoom();
	~OneToOneRoom();
	void onNewMessage(const std::wstring& sender, )

private:
	ui::Dialog ui;
};