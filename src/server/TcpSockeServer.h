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
* �ṹ�����ƣ�PER_HANDLE_DATA
* �ṹ��洢����¼�����׽��ֵ����ݣ��������׽��ֵı������׽��ֵĶ�Ӧ�Ŀͻ��˵ĵ�ַ��
* �ṹ�����ã��������������Ͽͻ���ʱ����Ϣ�洢���ýṹ���У�֪���ͻ��˵ĵ�ַ�Ա��ڻطá�
**/
typedef struct
{
	SOCKET socket;
	SOCKADDR_STORAGE ClientAddr;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

// ����ȫ�ֱ���
const int DefaultPort = 6543;
		// ��¼�ͻ��˵�������






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