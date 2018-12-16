#include "TcpSockeServer.h"
#include <assert.h>



bool CTCPSocketServer::InitNetwork()
{
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	DWORD err = WSAStartup(wVersionRequested, &wsaData);

	if (0 != err) {	// ����׽��ֿ��Ƿ�����ɹ�
		cerr << "Request Windows Socket Library Error!\n";
		system("pause");
		return false;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {// ����Ƿ�����������汾���׽��ֿ�
		WSACleanup();
		cerr << "Request Windows Socket Version 2.2 Error!\n";
		system("pause");
		return false;
	}
	return true;
}



CTCPSocketServer::CTCPSocketServer(char* ip, int port)
{
	//������ɶ˿�
	m_completionPort = CreateCompletionPort();
	assert(m_completionPort);

	//����socket
	m_sockServer = CreateSocket(ip, port);
	assert(m_sockServer > 0);
}

CTCPSocketServer::~CTCPSocketServer()
{

}

void CTCPSocketServer::MainTask()
{
	HANDLE ThreadHandle = CreateThread(NULL, 0, AcceptThreadFun, this, 0, NULL);
	// ȷ���������ĺ�������
	SYSTEM_INFO mySysInfo;
	GetSystemInfo(&mySysInfo);
	int iNumProcessors = mySysInfo.dwNumberOfProcessors;
	// ���ڴ������ĺ������������߳�
	for (DWORD i = 0; i < (1); ++i) {
		// �����������������̣߳�������ɶ˿ڴ��ݵ����߳�
		HANDLE ThreadHandle = CreateThread(NULL, 0, RecvMsgThread, this, 0, NULL);
		if (NULL == ThreadHandle) {
			cerr << "Create Thread Handle failed. Error:" << GetLastError() << endl;
			system("pause");
			return ;
		}
		CloseHandle(ThreadHandle);
	}
	
	return ;
}

SOCKET CTCPSocketServer::GetServerSocket() const
{
	return m_sockServer;
}

HANDLE CTCPSocketServer::GetCompletePort() const
{
	return m_completionPort;
}

void CTCPSocketServer::AddClientMsg(SOCKET sock, const string& msg)
{
	m_mapCient[sock] += msg;
}

void CTCPSocketServer::RemoveClient(SOCKET sock)
{
	map< SOCKET, string >::iterator iter = m_mapCient.begin();
	if (iter != m_mapCient.begin())
	{
		m_mapCient.erase(iter);
	}
}

void CTCPSocketServer::ClearClientMsg(SOCKET sock)
{
	map< SOCKET, string >::iterator iter = m_mapCient.find(sock);
	if (iter != m_mapCient.end())
	{
		iter->second.clear();
	}
}

std::map< SOCKET, std::string > CTCPSocketServer::GetAllClientMsg()
{
	return m_mapCient;
}

HANDLE CTCPSocketServer::CreateCompletionPort()
{
	HANDLE completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == completionPort) {	// ����IO�ں˶���ʧ��
		cerr << "CreateIoCompletionPort failed. Error:" << GetLastError() << endl;
		system("pause");
		return NULL;
	}
	return completionPort;
}

// ��ʼ�������̺߳���
DWORD WINAPI CTCPSocketServer::RecvMsgThread(LPVOID lpParam)
{
	CTCPSocketServer* sock = reinterpret_cast<CTCPSocketServer*>(lpParam);
	assert(sock);
	HANDLE CompletionPort = sock->GetCompletePort();
	DWORD BytesTransferred;
	LPOVERLAPPED IpOverlapped;
	LPPER_HANDLE_DATA PerHandleData = NULL;
	LPPER_IO_DATA PerIoData = NULL;
	DWORD RecvBytes;
	DWORD Flags = 0;
	BOOL bRet = false;

	while (true) {
		//��������ֱ�����ܵ���Ϣ
		bRet = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&IpOverlapped, INFINITE);
		assert(PerHandleData);
		assert(IpOverlapped);
		SOCKET iClient = PerHandleData->socket;

		if (bRet == 0) {
		 	cerr << "a client leave : " << iClient << endl;
			sock->RemoveClient(PerHandleData->socket);
		}

		PerIoData = (LPPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, overlapped);

		string strRecv = PerIoData->databuff.buf;
		// ������׽������Ƿ��д�����
		if (0 == BytesTransferred) {
			closesocket(PerHandleData->socket);
			GlobalFree(PerHandleData);
			GlobalFree(PerIoData);
			continue;
		}
		sock->AddClientMsg(iClient, strRecv);
		// ��ʼ���ݴ����������Կͻ��˵�����
		cout << iClient <<" says: " << strRecv.c_str() << endl;

		// Ϊ��һ���ص����ý�����I/O��������
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // ����ڴ�
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;	// read
		WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}

	return 0;
}

DWORD WINAPI CTCPSocketServer::AcceptThreadFun(LPVOID IpParam)
{
	CTCPSocketServer* sock = (CTCPSocketServer*)(IpParam);
	SOCKET sockServer = sock->GetServerSocket();
	HANDLE completionPort = sock->GetCompletePort();
	while (1) {
		PER_HANDLE_DATA * PerHandleData = NULL;
		SOCKADDR_IN saRemote;
		int RemoteLen;
		SOCKET acceptSocket;

		// �������ӣ���������ɶˣ����������AcceptEx()
		RemoteLen = sizeof(saRemote);
		acceptSocket = accept(sockServer, (SOCKADDR*)&saRemote, &RemoteLen);
		if (SOCKET_ERROR == acceptSocket) {	// ���տͻ���ʧ��
			cerr << "Accept Socket Error: " << GetLastError() << endl;
			system("pause");
			return -1;
		}

		// �����������׽��ֹ����ĵ����������Ϣ�ṹ
		PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));	// �ڶ���Ϊ���PerHandleData����ָ����С���ڴ�
		PerHandleData->socket = acceptSocket;
		memcpy(&PerHandleData->ClientAddr, &saRemote, RemoteLen);


		// �������ͻ�������ָ��ŵ��ͻ�������
		cerr << "a new client come : " << PerHandleData->socket<<endl;
		sock->AddClientMsg(PerHandleData->socket,"");
													// �������׽��ֺ���ɶ˿ڹ��������Ǵ���
		CreateIoCompletionPort((HANDLE)(PerHandleData->socket), completionPort, (DWORD)PerHandleData, 0);


		// ��ʼ�ڽ����׽����ϴ���I/Oʹ���ص�I/O����
		// ���½����׽�����Ͷ��һ�������첽
		// WSARecv��WSASend������ЩI/O������ɺ󣬹������̻߳�ΪI/O�����ṩ����	
		// ��I/O��������(I/O�ص�)
		LPPER_IO_OPERATION_DATA PerIoData = NULL;
		PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;	// read

		DWORD RecvBytes;
		DWORD Flags = 0;
		//ִ�����ݽ��յĲ���  �첽
		WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}
	return 0;
}

SOCKET CTCPSocketServer::CreateSocket(char* ip, int port)
{
	// ������ʽ�׽���
	SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, 0,NULL,0,WSA_FLAG_OVERLAPPED);
	//SOCKET sock = socket(AF_INET, SOCK_STREAM,0);
	// ��SOCKET
	SOCKADDR_IN srvAddr;
	srvAddr.sin_addr.S_un.S_addr =  inet_addr(ip);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(port);
	int bindResult = bind(sock, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
	if (SOCKET_ERROR == bindResult) {
		cerr << "Bind failed. Error:" << GetLastError() << endl;
		system("pause");
		return -1;
	}

	// ��SOCKET����Ϊ����ģʽ
	int listenResult = listen(sock, 10);
	if (SOCKET_ERROR == listenResult) {
		cerr << "Listen failed. Error: " << GetLastError() << endl;
		system("pause");
		return -2;
	}

	// �������ڷ������ݵ��߳�
	//HANDLE sendThread = CreateThread(NULL, 0, ServerSendThread, 0, 0, NULL);
	cerr << "create socket succ : " << port << endl;
	return sock;
}

