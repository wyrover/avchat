#ifndef QTCHATCLIENT_H
#define QTCHATCLIENT_H

#include <QtWidgets/QMainWindow>
#include "ui_qtchatclient.h"

class qtchatclient : public QMainWindow
{
	Q_OBJECT

public:
	qtchatclient(QWidget *parent = 0);
	~qtchatclient();

private:
	Ui::qtchatclientClass ui;
};

#endif // QTCHATCLIENT_H
