#include "MyTask.h"
#include "TcpSockeServer.h"


MyTask::MyTask()
{
	
}

MyTask::~MyTask()
{
}

void MyTask::MainTask()
{
	CTCPSocketServer::InitNetwork();
	CTCPSocketServer* server = new CTCPSocketServer("127.0.0.1",13579);
	server->MainTask();
	return ;

}

