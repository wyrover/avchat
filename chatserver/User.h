#pragma once



class User
{
public:
	enum UserStatus {
		kStatus_Invalid,
		kStatus_Online,
		kStatus_Offline,
	};
	User();
	~User();
	int	login(const std::wstring& email, const std::wstring& password);
	int requestAddFriend(int userId);
	int requestAddGroup(int groupId);
	int removeFriend(int userId);
	int quitGroup(int groupId);
	int logOff();
	int getStatus();

private:
	int createPreparedStatement();

private:
	int id_;
	std::wstring email_;
	std::wstring username_;
	//std::wstring accessToken_;
	int status_;
	static std::unique_ptr<sql::PreparedStatement> loginStmt_;
	static std::unique_ptr<sql::PreparedStatement> statusStmt_;
};