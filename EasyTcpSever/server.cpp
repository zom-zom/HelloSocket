#define NOMINMAX // ������д����Խ���min��max��
#include <iostream>
#include <thread>
#include <limits>
using namespace std;
#include "EasyTcpServer.hpp"

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
            g_bRun = false;
            cout << "cmdThread�߳��˳�" << endl;
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
    EasyTcpServer server;
    server.InitSocket();
    server.BindPort(nullptr, 4567);
    server.ListenPort(5);

    //�����̺߳���
    thread t1(cmdThread);
    t1.detach();

    while (g_bRun)
    {
        server.OnRun();

    }
    server.Close();
    cout << "���˳����������" << endl;
    
    getchar();
    return 0;
}
