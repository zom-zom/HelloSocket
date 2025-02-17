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

//��������С��Ԫ��С
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
	int getLast()
	{
		return _lastPos;
	}
	void setLast(int pos)
	{
		_lastPos = pos;
	}
private:
	//fd_set file description set 
	SOCKET _sockfd;
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	//��Ϣ������β����λ��
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
	//��ʼ��Socket
	SOCKET InitSocket()
	{
		//����Win Sock 2.x����
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		//��Socket API����һ��TCP�ͻ���
		//����һ��socket
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>�رվ�����\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			cout << "ERROR ��������Ч��_socket" << endl;
		}
		else
			cout << "����socket �ɹ�" << endl;
		return _sock;
	}
	//��IP�Ͷ˿ں�
	int BindPort(const char* ip,unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		///2.�����ڽ��տͻ������ӵ�����˿�
		sockaddr_in _sin = {}; //����+��_in���� ������д�������еı���
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); //host to net unsigned short
#ifdef _WIN32
		if (ip)
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);//any�����õ���������һ��ip����Ϊ�����ip��ַ\
																							inet_addr("127.0.0.1")����д����һ����ַ
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
			cout << "ERROR �����ڽ��ܿͻ������ӵ�����˿�ʧ��" << endl;
		else
			cout << "SUCCEED ������˿ڳɹ�" << endl;
		return ret;
	}
	//�����˿ں�
	int ListenPort(int nPort)
	{
		///listen ��������˿�
		//nPortΪͬʱ���������˿ڱ���
		int ret = listen(_sock, nPort);
		if (SOCKET_ERROR == ret)//listen()�ڶ�����Ϊ�ȴ�������ͬʱ����
			printf("<socket=%d>ERROR ��������˿�ʧ��\n", _sock);
		else
			printf("<socket=%d>SUCCEED ��������˿ڳɹ�\n", _sock);
		return ret;
	}
	//���ܿͻ�������
	SOCKET Accept()
	{
		///accept �ȴ����ܿͻ�������
		sockaddr_in clientAddr = {};//���ڽ��տͻ��˵�ַ��
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;//�����ܵĿͻ����ȳ�ʼ��Ϊ��Ч��
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif	
		if (INVALID_SOCKET == cSock)
		{
			printf("<socket=%d>ERROR, ���ܵ���Ч�ͻ���SOCKET", (int)_sock);
		}
		else
		{
			NewUserJoin userjoin;
			SendDataToAll(&userjoin);
			_clients.push_back(new ClientSocket(cSock));
			printf("<socket=%d>�¿ͻ��˼���:socket= %d IP = %s\n", (int)_sock, (int)cSock, inet_ntoa(clientAddr.sin_addr));//inet_ntoa()��ip��ַת��Ϊ�ɶ��ַ���
		}
		return cSock;
	}
	//�ر�Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			//�رս��յ�socket �׽���
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			///�ر��׽��� closesocket
			closesocket(_sock);
			WSACleanup();
#else
			//�رս��յ�socket �׽���
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
				delete _clients[n];
			}
			///8.�ر��׽��� closesocket
			close(_sock);
#endif
			_clients.clear();
		}
	}
	//����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			//�������׽��� BSD Socket
			fd_set fdRead;//������(socket) ����
			fd_set fdWrite;
			fd_set fdExp;
			//������
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			//����������socket�����뼯����
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);

			//���sockֵ
			SOCKET maxSock = _sock;
			//�����յ�����������socket������ ������
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock < _clients[n]->sockfd())
				{
					maxSock = _clients[n]->sockfd();
				}
			}
			//nfds ��һ������ֵ����ָfd_set�����е�������(socket)�ķ�Χ������������
			//���������ļ����������ֵ+1 ��Windows�������������д0
			timeval t = { 0,0 };//����ȴ��������ܲ�ѯ��   ������һ������NULL���Ƕ���ʽ
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			//cout << "����ʱ����������" << endl;
			if (ret < 0)
			{
				cout << "select�������" << endl;
				Close();
				return false;
			}
			//�ж���������socket���Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			//this method can using difference platform  include WIN32
			//ѭ���������� �ɶ�����������socket��
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
				{

					if (-1 == RecvData(_clients[n]))
					{
						//std::vector<SOCKET>::iterator iter = _clients.begin();
						auto iter = _clients.begin();
						if (iter != _clients.end())
						{
							delete _clients[n];
							_clients.erase(iter);
						}
					}
				}
			}
			return true;
		}
		return false;
	}
	//�Ƿ�����
	bool isRun()
	{
		return INVALID_SOCKET != _sock;
	}
	//����������
	char szRecv[1024] = {};
	//�������� ������ ��ְ�
	int RecvData(ClientSocket* pClient)
	{

		///���տͻ�������
		int nLen = (int)recv(pClient->sockfd(), szRecv, RECV_BUFF_SIZE, 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
			printf("�ͻ���<socket=%/*d>���˳����������\n", pClient->sockfd());
			return -1;
		}
		////��Ϊ��һ�ν���ͷʱ�Ѿ�ռ����ǰ����ֽڲ�������  ������Ҫƫ�Ʋ����������� ����Ҳ�ü�
		//recv(_cSock, szRecv + sizeof(DataHeader), header->dataLenth - sizeof(DataHeader), 0);
		//����ȡ�������ݿ�������Ϣ������
		memcpy(pClient->msgBuf() + pClient->getLast(), szRecv, nLen);
		OnNetMsg(pClient->sockfd(), header);
		return 0;
	}
	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET _cSock, DataHeader* header)
	{
		///��������
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;

			cout << "-----------------------------" << endl;
			printf("�յ��ͻ���<socket=%d>����: CMD_LOGIN \n���ݳ���: %d\n�û���: %s\n����:%s\n",
				_cSock, login->dataLenth, login->UserName, login->PassWord);
			cout << "-----------------------------" << endl;

			//�ж��û������Ƿ���ȷ�Ĺ���
			if (0 == strcmp(login->PassWord, "���Ǻؿ���"))
			{
				LoginResult _result;
				//send(_cSock, (const char*)&_result, sizeof(LoginResult), 0);
				SendData(_cSock, (DataHeader*)&_result);
				
			}
			else
			{
				LoginResult ret;
				ret.result = MyFailed;
				
				//send(_cSock, (const char*)&ret, sizeof(LoginResult), 0);
				SendData(_cSock, (DataHeader*)&ret);
			}
			
			break;
		}
		case CMD_LOGINOUT:
		{

			LoginOut* loginout = (LoginOut*)header;
			
			cout << "-----------------------------" << endl;
			printf("�յ��ͻ���<socket=%d>����: CMD_LOGINOUT \n���ݳ���: %d\n�û���: %s\n",
				_cSock, loginout->dataLenth, loginout->UserName);
			cout << "-----------------------------" << endl;
			
			//�ж��û��Ƿ���ȷ�Ĺ��� 
			if (0 == strcmp("tyzhou", loginout->UserName))
			{
				LoginOutResult ret;
				send(_cSock, (char*)&ret, sizeof(LoginOutResult), 0);
			}
			else
			{
				LoginOutResult ret;
				ret.result = MyFailed;
				send(_cSock, (char*)&ret, sizeof(LoginOutResult), 0);
			}
			
			break;
		}
		default:
		{
			DataHeader header = { 0,CMD_ERROR };
			send(_cSock, (const char*)&header, sizeof(DataHeader), 0);
		}
		break;
		}
	}
	//����ָ��Socket����
	int SendData(SOCKET _sock, DataHeader* header)
	{
		if (isRun() && header)
		{
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