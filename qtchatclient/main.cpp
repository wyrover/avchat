#include "qtchatclient.h"
#include "LoginDialog.h"
#include "../chatclient/ChatClient.h"
#include <QtWidgets/QApplication>
#include <assert.h>
#include <memory.>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	int count = 10000;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	assert(err == 0);
	std::unique_ptr<ChatClient> client(new ChatClient(L"127.0.0.1", 2333));
	LoginDialog dlg(client.get());
	auto result = dlg.exec();
	if (result == QDialog::Rejected)
		return 1;


	qtchatclient w(client.get());
	w.show();
	int ret = a.exec();
	WSACleanup();
	return ret;
}