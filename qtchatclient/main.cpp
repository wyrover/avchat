#include "qtchatclient.h"
#include "LoginDialog.h"
#include "../chatclient/ChatClient.h"
#include <QtWidgets/QApplication>
#include <QTranslator>
#include <assert.h>
#include <QDebug>
#include <memory>
#include "DropShadowWidget.h"


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	qRegisterMetaType<int64_t>();
	std::unique_ptr<ChatClient> client(new ChatClient());
	client->init(L"127.0.0.1", 2333);
	QTranslator appTranslator;
	appTranslator.load("qtchatclient_zh.qm", qApp->applicationDirPath());
	a.installTranslator(&appTranslator);
	LoginDialog dlg(client.get());
	auto result = dlg.exec();
	if (result == QDialog::Rejected)
		return 1;
	//QApplication::setQuitOnLastWindowClosed(false);
	qtchatclient w(client.get());
	w.show();
	return a.exec();
}