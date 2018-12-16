#ifndef _H_TASK_MANAGE_
#define _H_TASK_MANAGE_

#include "common.h"

class CTCPSocketServer;

class MyTask
{
public:
	MyTask();
	~MyTask();
	void MainTask();
	
private:
	map<int, vector<string> > m_mapRecvData;
	map<int, vector<string> > m_mapRequestData;
	CTCPSocketServer* m_sockServer;
	static void* ThreadProc(LPVOID lpParam);
	
};


#endif