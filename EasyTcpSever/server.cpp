
using namespace std;
#include "EasyTcpServer.hpp"


//���ڹر����߳�whileѭ���ı���
bool g_bRun = true;
void cmdThread(EasyTcpServer& server)
{
	char cmd_Buf[256] = {};
	scanf("%s", cmd_Buf);
	if (0 == strcmp("exit", cmd_Buf))
	{
		server.setsockError();
	}
}

int main()
{
	
	EasyTcpServer server;
	//EasyTcpServer server2;
	server.InitSocket();
	server.BindPort(nullptr, 4567);
	server.ListenPort(5);

	//�����̺߳���
	thread t1(cmdThread, ref(server));
	t1.detach();

	while (server.isRun())
	{
		server.OnRun();
	}
	server.Close();
	cout << "���˳����������" << endl;

	///

	getchar();
	return 0;


}

