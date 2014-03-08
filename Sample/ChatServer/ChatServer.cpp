// ChatServer.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "SFEngine.h"
#include "ChatLogicJsonEntry.h"
#include "SFJsonProtocol.h"

int _tmain(int argc, _TCHAR* argv[])
{

////////////////////////////////////////////////////////////////////
//Game Mode
////////////////////////////////////////////////////////////////////
	ChatLogicJsonEntry* pLogicEntry = new ChatLogicJsonEntry();
	
/////////////////////////////////////////////////////////////////////
	if (FALSE == SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SFJsonProtocol>) ||
		FALSE == SFEngine::GetInstance()->Start())
	{
		google::FlushLogFiles(google::GLOG_INFO);
		google::FlushLogFiles(google::GLOG_ERROR);
	
		return 0;
	}

	google::FlushLogFiles(google::GLOG_INFO);

	getchar();
	SFEngine::GetInstance()->ShutDown();

	return 0;
}