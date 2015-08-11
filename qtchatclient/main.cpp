#include "qtchatclient.h"
#include "LoginDialog.h"
#include "../chatclient/ChatClient.h"
#include <QtWidgets/QApplication>
#include <assert.h>
#include <memory.>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	qRegisterMetaType<int64_t>();
	std::unique_ptr<ChatClient> client(new ChatClient());
	client->init(L"127.0.0.1", 2333);
	LoginDialog dlg(client.get());
	auto result = dlg.exec();
	if (result == QDialog::Rejected)
		return 1;

	qtchatclient w(client.get());
	w.show();
	return a.exec();
}