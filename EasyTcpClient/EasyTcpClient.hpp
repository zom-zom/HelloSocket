#ifndef _EasyTcpClient_hpp_
#define  _EasyTcpClient_hpp_
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <WinSock2.h>

#else
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif
#include <iostream>
#include "MessageHeader.hpp"
#pragma comment(lib,"ws2_32.lib")
using namespace std;

class EasyTcpClient
{
	SOCKET _sock;
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	}

	virtual ~EasyTcpClient()
	{
		Close();
	}
	//测试
	void setsockError()
	{
		_sock = INVALID_SOCKET;
	}
	//初始化socket
	void InitSocket()
	{
		//启动Win Sock 2.x环境
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		//用Socket API建立一个TCP客户端
		//建立一个socket
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>关闭旧连接\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			cout << "ERROR 建立了无效的_socket" << endl;
		}
		else
			cout << "建立Socket 成功" << endl;
	}
	//连接服务器
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		//2.连接服务器
		sockaddr_in _sin = {}; //后面+个_in类型 方便手写此类型中的变量
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); //host to net unsigned short
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr));
		if (SOCKET_ERROR == ret)
			printf("<socket=%d>连接服务器<%s:%d>失败...\n", _sock, ip, port);
		else
			printf("<socket=%d>连接服务器<%s:%d>成功...\n", _sock, ip, port);
		return ret;
	}
	//关闭socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			//关闭套接字 closesocket
			closesocket(_sock);
			//清除Windows socket环境 
			WSACleanup();
#else
			close(_sock);
#endif
		}

	}
	
	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdRead;
			FD_ZERO(&fdRead);
			FD_SET(_sock, &fdRead);
			timeval t = { 1,0 };
			int ret = select(_sock + 1, &fdRead, 0, 0, &t);
			if (ret < 0)
			{
				cout << "select任务结束" << endl;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				//处理请求命令
				if (-1 == RecvData(_sock))
				{
					cout << "select任务结束2" << endl;
				}
				fd_set fdRead;
				FD_ZERO(&fdRead);
				FD_SET(_sock, &fdRead);
				timeval t = { 1,0 };
				int ret = select(_sock + 1, &fdRead, 0, 0, &t);
				if (ret < 0)
				{
					cout << "select任务结束" << endl;
				}
				if (FD_ISSET(_sock, &fdRead))
				{
					FD_CLR(_sock, &fdRead);
					//5.处理请求命令
					if (-1 == RecvData(_sock))
					{
						cout << "select任务结束2" << endl;
					}
				}

			}
			return true;
		}
		return false;
	}
	//缓冲区最小单元大小
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif
	//接收缓冲区
	char _szRecv[RECV_BUFF_SIZE] = {};
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	//消息缓冲区尾部的位置
	int _lastPos = 0;

	//接收数据 处理粘包 少包
	int RecvData(SOCKET cSock)
	{
		//接收服务端数据
		int nLen = (int)recv(cSock, _szRecv, sizeof(DataHeader), 0);
		if (nLen <= 0)
		{
			printf("与服务端<socket=%d>断开连接，任务结束\n", cSock);
			return -1;
		}
		//将收取到的数据拷贝到消息缓冲区
		memcpy(_szMsgBuf + _lastPos, _szRecv, nLen);
		//消息缓冲区的数据尾部位置后移
		_lastPos += nLen;
		//判断消息缓冲区的数据长度大于消息头DataHeader长度
		while (_lastPos >= sizeof(DataHeader))
		{
			//这时就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)_szMsgBuf;
			//判断消息缓冲区的数据长度大于消息长度
			if (_lastPos >= header->dataLenth)
			{
				//消息缓冲区剩余未处理数据的长度
				int nSize = _lastPos - header->dataLenth;
				//处理网络消息
				OnNetMsg(header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLenth, nSize);
				//消息缓冲区的数据尾部位置前移
				_lastPos = nSize;

			}
			else
			{
				//消息缓冲区剩余数据不够一条完整消息
				break;
			}
		}
		return 0;
		
	}
	//响应网络消息
	void OnNetMsg(DataHeader* header)
	{
		//处理请求
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			
			LoginResult* loginret = (LoginResult*)header;

			cout << "-----------------------------" << endl;
			printf("<socket=%d>收到服务端命令: CMD_LOGIN_RESULT 数据长度: %d\n返回值: %d\n",
				_sock, loginret->dataLenth, loginret->result);
			cout << "-----------------------------" << endl;
			if (MySuceed == loginret->result)
				cout << "登陆成功" << endl;
			else if (MyFailed == loginret->result)
				cout << "登陆失败" << endl;
		}
		break;
		case CMD_LOGINOUT_RESULT:
		{
			LoginOutResult* loginoutret = (LoginOutResult*)header;

			cout << "-----------------------------" << endl;
			printf("<socket=%d>收到服务端命令: CMD_LOGINOUTR_RESULT 数据长度: %d\n返回值: %d\n",
				_sock, loginoutret->dataLenth, loginoutret->result);
			cout << "-----------------------------" << endl;
			if (MySuceed == loginoutret->result)
				cout << "退出成功" << endl;
			else if (MyFailed == loginoutret->result)
				cout << "退出失败" << endl;
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userjoin = (NewUserJoin*)header;
			//printf("<socket=%d>收到服务端命令: CMD_NEW_USER_JOIN 数据长度: %d\n加入的新客户端: %d\n",
			//	_sock, userjoin->dataLenth, userjoin->sock);
		}
		break;
		case CMD_ERROR:
		{
			printf("<socket=%d>收到服务端命令: CMD_ERROR 数据长度: %d\n",
				_sock, header->dataLenth);
		}
		break;
		default:
		{
			printf("<socket=%d>收到未定义消息, 数据长度: %d\n",
				_sock, header->dataLenth); 
		}
		}
	}
	//是否在工作中
	bool isRun()
	{
		
		return INVALID_SOCKET != _sock;
	}
	//发送数据
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLenth, 0);
		}
		return SOCKET_ERROR;
	}

};












#endif