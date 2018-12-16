#pragma once
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <map>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
class MyTcpServer
{
public:
	MyTcpServer();
	~MyTcpServer();
	static int initNet();
	int initSocket(char* ip,const int &port);
	int Listen(const int &num);
	friend DWORD WINAPI ThreadFunAccept(LPVOID param);
	friend DWORD WINAPI ThreadFunRecv(LPVOID param);
private:
	void openThreads();
private:
	SOCKET m_sockSrv;
	map<int, vector<string>>m_map_requests;
	vector<SOCKET> m_sockConn;
};

