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
		kCommandType_FileRequest,
		kCommandType_FileRequestAck,
		kCommandType_FileTransfer,
		kCommandType_ImageMessageClient,
		kCommandType_ImageMessage,
		kCommandType_FileExists,
		kCommandType_FileExistsAck,
	};
}