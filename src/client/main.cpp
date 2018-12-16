#include <WinSock2.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <time.h>
#include <io.h>
#include <vector>

using namespace std;
#pragma comment(lib, "ws2_32.lib")

void main()
{
	//�����׽���
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Failed to load Winsock");
		return;
	}

	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(13579);
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); 
	//�����׽���
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == sockClient){
		printf("Socket() error:%d", WSAGetLastError());
		return;
	}

	//�������������������
	if (connect(sockClient, (struct  sockaddr*)&addrSrv, sizeof(addrSrv)) == INVALID_SOCKET){
		printf("Connect failed:%d", WSAGetLastError());
		return;
	}
	while(1)
	{
		printf("press enter:\n");
		getchar();
		string filedata = "i am client";
		send(sockClient, filedata.c_str(), filedata.length(), 0);
		//�ȴ�����
	/*	char strRecv[100] = { 0 };
		recv(sockClient, strRecv, sizeof(strRecv), 0);
		printf("recv msg:%s\n", strRecv);*/
	

	}
	//�ر��׽���
	//closesocket(sockClient);
	//WSACleanup();
	system("pause");
}
