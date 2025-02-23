#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unistd.h> //uni std
#include <arpa/inet.h>
#include <string.h>
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif
#include <iostream>
#include <thread>
#include <vector>
#include "MessageHeader.hpp"

//缓冲区最小单元大小
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif
class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}
	char* msgBuf()
	{
		return _szMsgBuf;
	}
	int getLastPos()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		_lastPos = pos;
	}
private:
	//fd_set file description set 
	SOCKET _sockfd;
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	//消息缓冲区尾部的位置
	int _lastPos;
};

class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	void setsockError()
	{
		_sock = INVALID_SOCKET;
	}
	//初始化Socket
	SOCKET InitSocket()
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
			cout << "建立socket 成功" << endl;
		return _sock;
	}
	//绑定IP和端口号
	int BindPort(const char* ip,unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		///2.绑定用于接收客户端连接的网络端口
		sockaddr_in _sin = {}; //后面+个_in类型 方便手写此类型中的变量
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); //host to net unsigned short
#ifdef _WIN32
		if (ip)
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);//any则是用电脑上任意一个ip都作为服务端ip地址\
																							inet_addr("127.0.0.1")此种写法绑定一个地址
		else
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
		if(ip)
			_sin.sin_addr.s_addr = inet_addr(ip);
		else
			_sin.sin_addr.s_addr = INADDR_ANY;
#endif 				
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
			cout << "ERROR 绑定用于接受客户端连接的网络端口失败" << endl;
		else
			cout << "SUCCEED 绑定网络端口成功" << endl;
		return ret;
	}
	//监听端口号
	int ListenPort(int nPort)
	{
		///listen 监听网络端口
		//nPort为同时接听几个端口变量
		int ret = listen(_sock, nPort);
		if (SOCKET_ERROR == ret)//listen()第二参数为等待几个人同时连接
			printf("<socket=%d>ERROR 监听网络端口失败\n", _sock);
		else
			printf("<socket=%d>SUCCEED 监听网络端口成功\n", _sock);
		return ret;
	}
	//接受客户端连接
	SOCKET Accept()
	{
		///accept 等待接受客户端连接
		sockaddr_in clientAddr = {};//用于接收客户端地址的
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;//将接受的客户端先初始化为无效的
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif	
		if (INVALID_SOCKET == cSock)
		{
			printf("<socket=%d>ERROR, 接受到无效客户端SOCKET", (int)_sock);
		}
		else
		{
			NewUserJoin userjoin;
			userjoin.sock = cSock;	// 设置新客户端的socket
			SendDataToAll(&userjoin);
			_clients.push_back(new ClientSocket(cSock));
			printf("<socket=%d>新客户端加入:socket= %d IP = %s\n", (int)_sock, (int)cSock, inet_ntoa(clientAddr.sin_addr));//inet_ntoa()将ip地址转换为可读字符串
		}
		return cSock;
	}
	//关闭Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			//关闭接收的socket 套接字
			for (int n = (int)_clients.size() - 1; n > 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				//delete _clients[n];

			}
			///关闭套接字 closesocket
			closesocket(_sock);
			_sock = INVALID_SOCKET;
			WSACleanup();
#else
			//关闭接收的socket 套接字
			for (int n = (int)_clients.size() - 1; n > 0; n--)
			{
				close(_clients[n]->sockfd());
				//delete _clients[n];
			}
			///8.关闭套接字 closesocket
			close(_sock);
#endif
			_clients.clear();
		}
	}
	//处理网络消息
	int _nCount = 0;
	bool OnRun()
	{
		if (isRun())
		{
			//伯克利套接字 BSD Socket
			fd_set fdRead;//描述符(socket) 集合
			fd_set fdWrite;
			fd_set fdExp;
			//清理集合
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			//将描述符（socket）加入集合中
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);

			//最大sock值
			SOCKET maxSock = _sock;
			//将接收到的描述符（socket）加入 集合中
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock < _clients[n]->sockfd())
				{
					maxSock = _clients[n]->sockfd();
				}
			}
			//nfds 是一个整数值，是指fd_set集合中的描述符(socket)的范围，而不是数量
			//既是所有文件描述符最大值+1 在Windows中这个参数可以写0
			timeval t = { 1,0 };//无需等待有数据能查询到   如果最后一个参数NULL则是堵塞式
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);

			if (ret < 0)
			{
				cout << "select任务结束" << endl;
				Close();
				return false;
			}
			//判断描述符（socket）是否在集合中
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			//this method can using difference platform  include WIN32
			//循环处理集合中 可读的描述符（socket）
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
				{

					if (-1 == RecvData(_clients[n]))
					{
						printf("客户端<socket=%d>已断开连接\n", _clients[n]->sockfd());
						auto iter = _clients.begin() + n;
						if (iter != _clients.end())
						{
							//delete _clients[n];
							_clients.erase(iter);
						}
					}
				}
			}
			return true;
		}
		return false;
	}
	//是否工作中
	bool isRun()
	{
		return INVALID_SOCKET != _sock;
	}
	//创建缓冲区
	char _szRecv[RECV_BUFF_SIZE] = {};
	//接收数据 处理黏包 拆分包
	int RecvData(ClientSocket* pClient)
	{

		///接收客户端数据
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		//printf("收到客户端数据 nlen = %d\n", nLen);
		if (nLen <= 0)
		{
			//printf("客户端<socket=%d>已退出，任务结束\n", pClient->sockfd());
			return -1;
		}

		DataHeader* header = (DataHeader*)_szRecv;

		//
		//将收取到的数据拷贝到消息缓冲区
		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);

		//消息缓冲区的数据尾部位置后移
		pClient->setLastPos(pClient->getLastPos() + nLen);

		//判断消息缓冲区的数据长度大于消息头DataHeader长度
		while (pClient->getLastPos() >= sizeof(DataHeader))
		{
			
			//这时就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)pClient->msgBuf();

			//判断消息缓冲区的数据长度大于消息长度
			if (pClient->getLastPos() >= header->dataLenth)
			{
				//消息缓冲区剩余未处理数据的长度
				int nSize = pClient->getLastPos() - header->dataLenth;

				//处理网络消息
				OnNetMsg(pClient->sockfd(), header);

				//将消息缓冲区剩余未处理数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLenth, nSize);

				//消息缓冲区的数据尾部位置前移
				pClient->setLastPos(nSize);
				//printf("LastPos %d  nSize = %d", pClient->getLastPos(), nSize);
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
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header)
	{
		///处理请求
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			//Login* login = (Login*)header;

			//cout << "-----------------------------" << endl;
			//printf("收到客户端<socket=%d>命令: CMD_LOGIN \n数据长度: %d\n用户名: %s\n密码:%s\n",
			//	cSock, login->dataLenth, login->UserName, login->PassWord);
			//cout << "-----------------------------" << endl;
			
			//忽略判断过程
			LoginResult ret;
			SendData(cSock, &ret);

			//判断用户密码是否正确的过程
			//if (0 == strcmp(login->PassWord, "我是贺俊铭"))
			//{
			//	LoginResult ret;	
			//	SendData(cSock, (DataHeader*)&ret);
			//}
			//else
			//{
			//	LoginResult ret;
			//	ret.result = MyFailed;
			//	SendData(cSock, (DataHeader*)&ret);
			//}
			
			break;
		}
		case CMD_LOGINOUT:
		{

			LoginOut* loginout = (LoginOut*)header;
			
			//cout << "-----------------------------" << endl;
			//printf("收到客户端<socket=%d>命令: CMD_LOGINOUT \n数据长度: %d\n用户名: %s\n",
			//	cSock, loginout->dataLenth, loginout->UserName);
			//cout << "-----------------------------" << endl;

			//判断用户是否正确的过程 
			//if (0 == strcmp("tyzhou", loginout->UserName))
			//{
			//	LoginOutResult ret;
			//	SendData(cSock, &ret);
			//}
			//else
			//{
			//	LoginOutResult ret;
			//	ret.result = MyFailed;
			//	SendData(cSock, &ret);
			//}
			
			break;
		}
		default:
		{
			printf("<socket=%d>收到未定义消息, 数据长度: %d\n", cSock, header->dataLenth);
			DataHeader ret;
			SendData(cSock, &ret);
		}
		break;
		}
	}
	//发送指定Socket数据
	int SendData(SOCKET _sock, DataHeader* header)
	{
		if (isRun() && header)
		{
			//printf("send a message, len = %d\n", header->dataLenth);
			return send(_sock, (const char*)header, header->dataLenth, 0);
		}
		return SOCKET_ERROR;
	}
	void SendDataToAll(DataHeader* header)
	{
		if (isRun() && header)
		{
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				SendData(_clients[n]->sockfd(), header);
			}
		}
	}

};

#endif