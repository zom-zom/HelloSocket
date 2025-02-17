#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "EasyTcpClient.hpp"
#include <thread>

using namespace std;
//���ڹر����߳�whileѭ���ı���
bool g_bRun = true;
//�߳�Thread
void cmdThread(EasyTcpClient &client)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		//strcpy(cmdBuf, "login");
		if (0 == strcmp(cmdBuf, "exit"))
		{
			cout << "cmdThread�߳��˳�" << endl;
			client.setsockError();
			break;
		}
		//4.�������������������
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.UserName, "���������Ǻؿ�����Ӣ���������");
			printf("���������Ǻؿ���\n����������: ");	//�û���:tyzhou
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
			cout << "��֧�ֵ�����" << endl;
		}
	}
}


int main()
{
	EasyTcpClient client;
	client.InitSocket();
	client.Connect("115.236.153.174", 19576);	//127.0.0.1

	//  1kt034em37362.vicp.fun:19576
	//�����̺߳���
	thread t1(cmdThread, ref(client));
	t1.detach();



	while (client.isRun())
	{
			client.OnRun();

	}

	client.Close();

	return 0;


}