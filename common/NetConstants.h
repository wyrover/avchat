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
		kCommandType_Logout,

		kCommandType_ImageMessage,
		kCommandType_FileTransferRequest,
		kCommandType_FileTransferRequestAck,
		kCommandType_BuildPath,
		kCommandType_BuildPathAck,
		kCommandType_PeerSync,
		kCommandType_PeerSyncAck,
	};

	enum FileType {
		kPng,
		kJpeg,
		kGif,
	};

	enum LoginType {
		kLoginType_Normal,
		kLoginType_Auto
	};

	enum LoginAckType {
		kLoginAck_Failed,
		kLoginAck_Succeeded,
	};
	enum BuildPathAckType {
		kP2pAck_ConnTcp, 
		kP2pAck_ListenTcp,
		kP2pAck_TcpHole,
		kP2pAck_UdpHole,
	};
}