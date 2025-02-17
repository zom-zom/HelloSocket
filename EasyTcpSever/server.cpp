
using namespace std;
#include "EasyTcpServer.hpp"


//用于关闭主线程while循环的变量
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

	//启动线程函数
	thread t1(cmdThread, ref(server));
	t1.detach();

	while (server.isRun())
	{
		server.OnRun();
	}
	server.Close();
	cout << "已退出，任务结束" << endl;

	///

	getchar();
	return 0;


}

