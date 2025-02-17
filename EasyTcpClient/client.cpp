#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "EasyTcpClient.hpp"
#include <thread>

using namespace std;
//用于关闭主线程while循环的变量
bool g_bRun = true;
//线程Thread
void cmdThread(EasyTcpClient &client)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		//strcpy(cmdBuf, "login");
		if (0 == strcmp(cmdBuf, "exit"))
		{
			cout << "cmdThread线程退出" << endl;
			client.setsockError();
			break;
		}
		//4.向服务器发送请求命令
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.UserName, "请输入你是贺俊铭用英语哈哈哈哈");
			printf("请输入你是贺俊铭\n请输入密码: ");	//用户名:tyzhou
			scanf("%s", login.PassWord);
			//strcpy(login.PassWord, "hdw123");
			client.SendData(&login);
			//send(_sock, (const char*)&login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmdBuf, "loginout"))
		{
			LoginOut loginout;
			strcpy(loginout.UserName, "tyzhou");
			client.SendData(&loginout);
			//send(_sock, (const char*)&loginout, sizeof(LoginOut), 0);
		}
		else
		{
			cout << "不支持的命令" << endl;
		}
	}
}


int main()
{
	EasyTcpClient client;
	client.InitSocket();
	client.Connect("115.236.153.174", 19576);	//127.0.0.1

	//  1kt034em37362.vicp.fun:19576
	//启动线程函数
	thread t1(cmdThread, ref(client));
	t1.detach();



	while (client.isRun())
	{
			client.OnRun();

	}

	client.Close();

	return 0;


}