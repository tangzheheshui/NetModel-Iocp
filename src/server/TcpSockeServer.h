#ifndef _H_TCP_SERVER_
#define _H_TCP_SERVER_
#include "common.h"




const int DataBuffSize = 2 * 1024;
typedef struct
{
	OVERLAPPED overlapped;
	WSABUF databuff;
	char buffer[DataBuffSize];
	int BufferLen;
	int operationType;
}PER_IO_OPERATEION_DATA, *LPPER_IO_OPERATION_DATA, *LPPER_IO_DATA, PER_IO_DATA;

/**
* 结构体名称：PER_HANDLE_DATA
* 结构体存储：记录单个套接字的数据，包括了套接字的变量及套接字的对应的客户端的地址。
* 结构体作用：当服务器连接上客户端时，信息存储到该结构体中，知道客户端的地址以便于回访。
**/
typedef struct
{
	SOCKET socket;
	SOCKADDR_STORAGE ClientAddr;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

// 定义全局变量
const int DefaultPort = 6543;
		// 记录客户端的向量组






class CTCPSocketServer
{
public:

	static bool InitNetwork();

	CTCPSocketServer(char* ip, int port);

	~CTCPSocketServer();

	void MainTask();

	SOCKET GetServerSocket()const;

	HANDLE GetCompletePort() const;

	void AddClientMsg(SOCKET sock,const string& msg);

	void RemoveClient(SOCKET sock);

	void ClearClientMsg(SOCKET sock);

	map< SOCKET, string > GetAllClientMsg();

private:

	HANDLE CreateCompletionPort();

	static DWORD WINAPI RecvMsgThread(LPVOID lpParam);

	static DWORD WINAPI AcceptThreadFun(LPVOID IpParam);

	SOCKET CreateSocket(char* ip, int port);

private:

	HANDLE m_completionPort;

	SOCKET m_sockServer;

	map< SOCKET,string > m_mapCient;
	

};



#endif