#include "TcpSockeServer.h"
#include <assert.h>



bool CTCPSocketServer::InitNetwork()
{
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	DWORD err = WSAStartup(wVersionRequested, &wsaData);

	if (0 != err) {	// 检查套接字库是否申请成功
		cerr << "Request Windows Socket Library Error!\n";
		system("pause");
		return false;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {// 检查是否申请了所需版本的套接字库
		WSACleanup();
		cerr << "Request Windows Socket Version 2.2 Error!\n";
		system("pause");
		return false;
	}
	return true;
}



CTCPSocketServer::CTCPSocketServer(char* ip, int port)
{
	//创建完成端口
	m_completionPort = CreateCompletionPort();
	assert(m_completionPort);

	//创建socket
	m_sockServer = CreateSocket(ip, port);
	assert(m_sockServer > 0);
}

CTCPSocketServer::~CTCPSocketServer()
{

}

void CTCPSocketServer::MainTask()
{
	HANDLE ThreadHandle = CreateThread(NULL, 0, AcceptThreadFun, this, 0, NULL);
	// 确定处理器的核心数量
	SYSTEM_INFO mySysInfo;
	GetSystemInfo(&mySysInfo);
	int iNumProcessors = mySysInfo.dwNumberOfProcessors;
	// 基于处理器的核心数量创建线程
	for (DWORD i = 0; i < (1); ++i) {
		// 创建服务器工作器线程，并将完成端口传递到该线程
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
	if (NULL == completionPort) {	// 创建IO内核对象失败
		cerr << "CreateIoCompletionPort failed. Error:" << GetLastError() << endl;
		system("pause");
		return NULL;
	}
	return completionPort;
}

// 开始服务工作线程函数
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
		//会阻塞，直到接受到消息
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
		// 检查在套接字上是否有错误发生
		if (0 == BytesTransferred) {
			closesocket(PerHandleData->socket);
			GlobalFree(PerHandleData);
			GlobalFree(PerIoData);
			continue;
		}
		sock->AddClientMsg(iClient, strRecv);
		// 开始数据处理，接收来自客户端的数据
		cout << iClient <<" says: " << strRecv.c_str() << endl;

		// 为下一个重叠调用建立单I/O操作数据
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // 清空内存
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

		// 接收连接，并分配完成端，这儿可以用AcceptEx()
		RemoteLen = sizeof(saRemote);
		acceptSocket = accept(sockServer, (SOCKADDR*)&saRemote, &RemoteLen);
		if (SOCKET_ERROR == acceptSocket) {	// 接收客户端失败
			cerr << "Accept Socket Error: " << GetLastError() << endl;
			system("pause");
			return -1;
		}

		// 创建用来和套接字关联的单句柄数据信息结构
		PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));	// 在堆中为这个PerHandleData申请指定大小的内存
		PerHandleData->socket = acceptSocket;
		memcpy(&PerHandleData->ClientAddr, &saRemote, RemoteLen);


		// 将单个客户端数据指针放到客户端组中
		cerr << "a new client come : " << PerHandleData->socket<<endl;
		sock->AddClientMsg(PerHandleData->socket,"");
													// 将接受套接字和完成端口关联，而非创建
		CreateIoCompletionPort((HANDLE)(PerHandleData->socket), completionPort, (DWORD)PerHandleData, 0);


		// 开始在接受套接字上处理I/O使用重叠I/O机制
		// 在新建的套接字上投递一个或多个异步
		// WSARecv或WSASend请求，这些I/O请求完成后，工作者线程会为I/O请求提供服务	
		// 单I/O操作数据(I/O重叠)
		LPPER_IO_OPERATION_DATA PerIoData = NULL;
		PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;	// read

		DWORD RecvBytes;
		DWORD Flags = 0;
		//执行数据接收的操作  异步
		WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}
	return 0;
}

SOCKET CTCPSocketServer::CreateSocket(char* ip, int port)
{
	// 建立流式套接字
	SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, 0,NULL,0,WSA_FLAG_OVERLAPPED);
	//SOCKET sock = socket(AF_INET, SOCK_STREAM,0);
	// 绑定SOCKET
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

	// 将SOCKET设置为监听模式
	int listenResult = listen(sock, 10);
	if (SOCKET_ERROR == listenResult) {
		cerr << "Listen failed. Error: " << GetLastError() << endl;
		system("pause");
		return -2;
	}

	// 创建用于发送数据的线程
	//HANDLE sendThread = CreateThread(NULL, 0, ServerSendThread, 0, 0, NULL);
	cerr << "create socket succ : " << port << endl;
	return sock;
}

