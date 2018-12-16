#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib,"ws2_32.lib")

void main()
{
	WSADATA wsaData;
	int port = 6006;
	char buf[] = "Server:hello ,I am a server";
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Failed to load Winsock");
		return;
	}
	//套接字
	SOCKET sockSrv = socket(AF_INET,SOCK_STREAM,0);
	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(port);
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	int retVal = bind(sockSrv,(LPSOCKADDR)&addrSrv,sizeof(SOCKADDR_IN));
	if (retVal == SOCKET_ERROR)
	{
		printf("Failed bind:%d\n",WSAGetLastError());
		return;
	}
	if (listen(sockSrv, 10) == SOCKET_ERROR)
	{
		printf("Listen failed:%d",WSAGetLastError());
		return;
	}
	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);
	while (1)
	{
		SOCKET sockConn = accept(sockSrv,(SOCKADDR*)&addrClient,&len);
		if (sockConn == SOCKET_ERROR)
		{
			printf("Accept failed:%d", WSAGetLastError());
			break;
		}
		printf("Accept client IP:[%s]\n",inet_ntoa(addrClient.sin_addr));
		//发送数据
		int iSend = send(sockConn,buf,sizeof(buf),0);
		if (iSend == SOCKET_ERROR)
		{
			printf("send failed");
			break;
		}
		char recvBuf[100];
		memset(recvBuf,0,sizeof(recvBuf));
		recv(sockConn,recvBuf,sizeof(recvBuf),0);
		printf("%s\n",recvBuf);
		closesocket(sockConn);

	}
	closesocket(sockSrv);
	WSACleanup();
	system("pause");
	return;
}