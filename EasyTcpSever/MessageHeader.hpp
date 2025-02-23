#ifndef _MessageHeader_hpp_

#define _MessageHeader_hpp_


#define MySuceed 0
#define MyFailed -1
enum CMD
{
	CMD_LOGIN,
	CMD_LOGINOUT,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct DataHeader
{
	DataHeader()
	{
		dataLenth = sizeof(DataHeader);
		cmd = CMD_ERROR;
	}
	short dataLenth;
	short cmd;
};
struct Login :public DataHeader
{
	Login()
	{
		dataLenth = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char UserName[32];
	char PassWord[32];
	char data[932];
};
struct LoginOut :public DataHeader
{
	LoginOut()
	{
		dataLenth = sizeof(LoginOut);
		cmd = CMD_LOGINOUT;
	}
	char UserName[32];
};
struct LoginResult :public DataHeader
{
	LoginResult()
	{
		dataLenth = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = MySuceed;
	}
	int result;
	char data[992];
};
struct LoginOutResult :public DataHeader
{
	LoginOutResult()
	{
		dataLenth = sizeof(LoginOutResult);
		cmd = CMD_LOGINOUT_RESULT;
		result = MySuceed;
	}

	int result;
};
struct NewUserJoin :public DataHeader
{
	NewUserJoin()
	{
		dataLenth = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	SOCKET sock;
};

#endif