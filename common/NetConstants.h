#pragma once
namespace net
{
	enum NetworkType {
		kNetType_Accept,
		kNetType_Send,
		kNetType_Recv,
	};

	enum CommandType {
		kCommandType_Login,
		kCommandType_LoginAck,
		kCommandType_Message,
		kCommandType_UserList,
	};
}