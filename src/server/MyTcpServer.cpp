#include "MyTcpServer.h"


MyTcpServer::MyTcpServer()
{
}


MyTcpServer::~MyTcpServer()
{
}
int MyTcpServer::initNet()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)
	{
		printf("sucess to load Winsock\n");
		return 0;
	}
	else
	{
		printf("fail to load Winsock\n");
		return -1;
	}
}
int MyTcpServer::initSocket(char* ip,const int& port)
{
	//套接字
	m_sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(port);
	addrSrv.sin_addr.S_un.S_addr = inet_addr(ip);
	int retVal = bind(m_sockSrv, (LPSOCKADDR)&addrSrv, sizeof(SOCKADDR_IN));
	if (retVal == SOCKET_ERROR)
	{
		printf("Failed bind:%d\n", WSAGetLastError());
		return -1;
	}
	else
	{
		printf("sucess to bind socket[%s:%d]\n", ip,port);
		return 0;
	}
}
int MyTcpServer::Listen(const int &num)
{
	if (listen(m_sockSrv, num) == SOCKET_ERROR)
	{
		printf("Listen failed:%d", WSAGetLastError());
		return -1;
	}
	else
	{
		printf("listening...\n");
		//开启连接线程
		openThreads();
		return 0;
	}
}
DWORD WINAPI ThreadFunAccept(LPVOID param)
{
	MyTcpServer* tcpserver = (MyTcpServer*)param;
	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);
	while (1)
	{
		
		//等待连接
		SOCKET sockConn1 = accept(tcpserver->m_sockSrv, (SOCKADDR*)&addrClient, &len);
		if (sockConn1 == SOCKET_ERROR)
		{
			printf("Accept failed:%d", WSAGetLastError());
			//break;
		}
		tcpserver->m_sockConn.push_back(sockConn1);
	}
	return 0;
}
DWORD WINAPI ThreadFunRecv(LPVOID param)
{
	MyTcpServer* tcpserver = (MyTcpServer*)param;
	while (1)
	{
		
		char recvBuf[100] = { 0 };
		memset(recvBuf, 0, sizeof(recvBuf));
		for (int i = 0; i < tcpserver->m_sockConn.size();i++)
		{
			string temp;
			recv(tcpserver->m_sockConn[i], recvBuf, sizeof(recvBuf), 0);
			temp.append(recvBuf);
			tcpserver->m_map_requests[tcpserver->m_sockConn[i]].push_back(temp);
			//printf("%d\n", m_sockConn[0]);
			
		}
	}
	return 0;
	
}
void MyTcpServer::openThreads()
{
	CreateThread(NULL, 0, ThreadFunAccept, this, 0, NULL);
	CreateThread(NULL, 0, ThreadFunRecv, this, 0, NULL);
	
}
