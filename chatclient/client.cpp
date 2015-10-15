#include "stdafx.h"
#include "ChatClient.h"

using namespace avc;
int main(int argc, char* argv[])
{
	ChatClient* clientA = nullptr;
	clientA = new ChatClient();
	clientA->init(u"127.0.0.1", 2333);
	clientA->login(u"jcyangzh", u"2264seen");
	return 0;
}
