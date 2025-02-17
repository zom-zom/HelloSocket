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
	//����
	void setsockError()
	{
		_sock = INVALID_SOCKET;
	}
	//��ʼ��socket
	void InitSocket()
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
			cout << "����Socket �ɹ�" << endl;
	}
	//���ӷ�����
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		//2.���ӷ�����
		sockaddr_in _sin = {}; //����+��_in���� ������д�������еı���
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); //host to net unsigned short
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr));
		if (SOCKET_ERROR == ret)
			printf("<socket=%d>���ӷ�����<%s:%d>ʧ��...\n", _sock, ip, port);
		else
			printf("<socket=%d>���ӷ�����<%s:%d>�ɹ�...\n", _sock, ip, port);
		return ret;
	}
	//�ر�socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			//�ر��׽��� closesocket
			closesocket(_sock);
			//���Windows socket���� 
			WSACleanup();
#else
			close(_sock);
#endif
		}

	}
	
	//����������Ϣ
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
				cout << "select�������" << endl;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				//������������
				if (-1 == RecvData(_sock))
				{
					cout << "select�������2" << endl;
				}
				fd_set fdRead;
				FD_ZERO(&fdRead);
				FD_SET(_sock, &fdRead);
				timeval t = { 1,0 };
				int ret = select(_sock + 1, &fdRead, 0, 0, &t);
				if (ret < 0)
				{
					cout << "select�������" << endl;
				}
				if (FD_ISSET(_sock, &fdRead))
				{
					FD_CLR(_sock, &fdRead);
					//5.������������
					if (-1 == RecvData(_sock))
					{
						cout << "select�������2" << endl;
					}
				}

			}
			return true;
		}
		return false;
	}
	//��������С��Ԫ��С
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif
	//���ջ�����
	char _szRecv[RECV_BUFF_SIZE] = {};
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	//��Ϣ������β����λ��
	int _lastPos = 0;

	//�������� ����ճ�� �ٰ�
	int RecvData(SOCKET cSock)
	{
		//���շ��������
		int nLen = (int)recv(cSock, _szRecv, sizeof(DataHeader), 0);
		if (nLen <= 0)
		{
			printf("������<socket=%d>�Ͽ����ӣ��������\n", cSock);
			return -1;
		}
		//����ȡ�������ݿ�������Ϣ������
		memcpy(_szMsgBuf + _lastPos, _szRecv, nLen);
		//��Ϣ������������β��λ�ú���
		_lastPos += nLen;
		//�ж���Ϣ�����������ݳ��ȴ�����ϢͷDataHeader����
		while (_lastPos >= sizeof(DataHeader))
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			DataHeader* header = (DataHeader*)_szMsgBuf;
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����
			if (_lastPos >= header->dataLenth)
			{
				//��Ϣ������ʣ��δ�������ݵĳ���
				int nSize = _lastPos - header->dataLenth;
				//����������Ϣ
				OnNetMsg(header);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLenth, nSize);
				//��Ϣ������������β��λ��ǰ��
				_lastPos = nSize;

			}
			else
			{
				//��Ϣ������ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}
		return 0;
		
	}
	//��Ӧ������Ϣ
	void OnNetMsg(DataHeader* header)
	{
		//��������
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			
			LoginResult* loginret = (LoginResult*)header;

			cout << "-----------------------------" << endl;
			printf("<socket=%d>�յ����������: CMD_LOGIN_RESULT ���ݳ���: %d\n����ֵ: %d\n",
				_sock, loginret->dataLenth, loginret->result);
			cout << "-----------------------------" << endl;
			if (MySuceed == loginret->result)
				cout << "��½�ɹ�" << endl;
			else if (MyFailed == loginret->result)
				cout << "��½ʧ��" << endl;
		}
		break;
		case CMD_LOGINOUT_RESULT:
		{
			LoginOutResult* loginoutret = (LoginOutResult*)header;

			cout << "-----------------------------" << endl;
			printf("<socket=%d>�յ����������: CMD_LOGINOUTR_RESULT ���ݳ���: %d\n����ֵ: %d\n",
				_sock, loginoutret->dataLenth, loginoutret->result);
			cout << "-----------------------------" << endl;
			if (MySuceed == loginoutret->result)
				cout << "�˳��ɹ�" << endl;
			else if (MyFailed == loginoutret->result)
				cout << "�˳�ʧ��" << endl;
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userjoin = (NewUserJoin*)header;
			//printf("<socket=%d>�յ����������: CMD_NEW_USER_JOIN ���ݳ���: %d\n������¿ͻ���: %d\n",
			//	_sock, userjoin->dataLenth, userjoin->sock);
		}
		break;
		case CMD_ERROR:
		{
			printf("<socket=%d>�յ����������: CMD_ERROR ���ݳ���: %d\n",
				_sock, header->dataLenth);
		}
		break;
		default:
		{
			printf("<socket=%d>�յ�δ������Ϣ, ���ݳ���: %d\n",
				_sock, header->dataLenth); 
		}
		}
	}
	//�Ƿ��ڹ�����
	bool isRun()
	{
		
		return INVALID_SOCKET != _sock;
	}
	//��������
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