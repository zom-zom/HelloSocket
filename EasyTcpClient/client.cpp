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
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			cout << "cmdThread�߳��˳�" << endl;
			g_bRun = false;
			break;
		}
		else
		{
			cout << "��֧�ֵ�����" << endl;
		}
	}
}


int main()
{
	//FD_SETSIZE - 1
	const int nCount = 1000;
	EasyTcpClient* client[nCount];
	//EasyTcpClient client;
	//client.InitSocket();
	//client.Connect("127.0.0.1", 4567);//192.168.68.128
	int a = sizeof(EasyTcpClient);
	for (int i = 0; i < nCount; i++)
	{
		client[i] = new EasyTcpClient();
		client[i]->InitSocket();
		client[i]->Connect("192.168.68.128", 4567);	//115.236.153.174  19576
	}


	//  1kt034em37362.vicp.fun:19576
	//�����̺߳���
	thread t1(cmdThread);
	t1.detach();

	Login login;
	strcpy(login.UserName, "tyzhou");
	strcpy(login.PassWord, "hdw123");

	while (g_bRun)
	{
		for (int i = 0; i < nCount; i++)
		{
			client[i]->SendData(&login);
			client[i]->OnRun();
		}
	}
	//client.Close();
	for (int i = 0; i < nCount; i++)
	{
		client[i]->Close();
		delete client[i];
	}
	return 0;


}