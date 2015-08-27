#include "stdafx.h"
#include "SimpleTest.h"
#include "Utils.h"
#include <assert.h>
#include "ServerContext.h"
#include "user.h"
#include "../common/NetConstants.h"

namespace {
	
}

SimpleTest::SimpleTest()
{
}


SimpleTest::~SimpleTest()
{
}

void SimpleTest::testPasswordHash()
{
	Utils::GeneratePasswordHash("keke", "DBF01344987763A2109938E1D4DAEEDE54A229143AD57635591A852A5295AE14");
	std::string hash;
	Utils::GeneratePasswordHash("keke", &hash);
	assert(Utils::ValidatePasswordHash("keke", hash));
}

void SimpleTest::testDB()
{
	ServerContext::getInstance()->init();
	User user;
	assert(user.login(net::kLoginType_Normal, L"���@gmail.com", L"keke"));
}