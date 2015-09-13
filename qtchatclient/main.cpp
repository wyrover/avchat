#include "qtchatclient.h"
#include <QtWidgets/QApplication>
#include <QTranslator>
#include <assert.h>
#include <QDebug>
#include <memory>
#include "LoginDialog.h"
#include "DropShadowWidget.h"
#include "../common/WSAStarter.h"
#include "../chatclient/ChatClient.h"


int main(int argc, char *argv[])
{
	WSAStarter starter;
	if (!starter.init())
		return -1;
	QApplication a(argc, argv);
	qRegisterMetaType<int64_t>();
	std::unique_ptr<avc::ChatClient> client(new avc::ChatClient());
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