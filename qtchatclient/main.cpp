#include "qtchatclient.h"

#include <assert.h>
#include <syslog.h>
#include <memory>

#include <QtWidgets/QApplication>
#include <QTranslator>
#include <QDebug>

#include "../chatclient/ChatClient.h"
#include "LoginDialog.h"
#include "DropShadowWidget.h"
#include "ChatClientController.h"

int main(int argc, char *argv[])
{
    openlog("qtchatclient", LOG_PID | LOG_CONS, LOG_USER);
    QApplication a(argc, argv);

	QTranslator appTranslator;
	appTranslator.load("qtchatclient_zh.qm", qApp->applicationDirPath());
    a.installTranslator(&appTranslator);

    auto controller = new ChatClientController();
    std::unique_ptr<avc::ChatClient> client(new avc::ChatClient());
    client->init(u"127.0.0.1", 2333);
    client->setController(controller);

	LoginDialog dlg(client.get());
	auto result = dlg.exec();
    if (result == QDialog::Rejected) {
        client->logout();
        return 1;
    }

    QApplication::setQuitOnLastWindowClosed(true);
	qtchatclient w(client.get());
	w.show();
    auto rc =  a.exec();
    closelog();
    return rc;
}
