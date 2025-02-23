#define NOMINMAX // 添加这行代码以禁用min和max宏
#include <iostream>
#include <thread>
#include <limits>
using namespace std;
#include "EasyTcpServer.hpp"

//用于关闭主线程while循环的变量
bool g_bRun = true;

//线程Thread
void cmdThread()
{
    while (true)
    {
        char cmdBuf[256] = {};
        scanf("%s", cmdBuf);
        if (0 == strcmp(cmdBuf, "exit"))
        {
            g_bRun = false;
            cout << "cmdThread线程退出" << endl;
            break;
        }
        else
        {
            cout << "不支持的命令" << endl;
        }
    }
}

int main()
{
    EasyTcpServer server;
    server.InitSocket();
    server.BindPort(nullptr, 4567);
    server.ListenPort(5);

    //启动线程函数
    thread t1(cmdThread);
    t1.detach();

    while (g_bRun)
    {
        server.OnRun();

    }
    server.Close();
    cout << "已退出，任务结束" << endl;
    
    getchar();
    return 0;
}
