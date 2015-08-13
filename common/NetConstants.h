#pragma once
namespace net
{
	enum ActionType {
		kAction_Accept,
		kAction_Send,
		kAction_Recv,
		kAction_SendMessage,
	};

	enum CommandType {
		kCommandType_Login,
		kCommandType_LoginAck,
		kCommandType_Message,
		kCommandType_MessageAck,
		kCommandType_UserList,
		kCommandType_UserListAck,
		kCommandType_FileExists,
		kCommandType_FileExistsAck,
		kCommandType_FileUpload,
		kCommandType_FileUploadAck,
		kCommandType_FileDownload,
		kCommandType_FileDownloadAck,

		kCommandType_ImageMessage,
		kCommandType_FileRequest,
		kCommandType_FileRequestAck,
	};

	enum FileType {
		kPng,
		kJpeg,
		kGif,
	};

}