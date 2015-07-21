#ifndef QTCHATSERVER_H
#define QTCHATSERVER_H

#include <QtWidgets/QMainWindow>
#include "ui_qtchatserver.h"

class qtchatserver : public QMainWindow
{
	Q_OBJECT

public:
	qtchatserver(QWidget *parent = 0);
	~qtchatserver();

private:
	Ui::qtchatserverClass ui;
};

#endif // QTCHATSERVER_H
