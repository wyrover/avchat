#include "qtchatserver.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	qtchatserver w;
	w.show();
	return a.exec();
}
