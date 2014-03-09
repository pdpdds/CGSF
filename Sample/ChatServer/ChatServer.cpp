// ChatServer.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "ChatLogicEntry.h"
#include "SFJsonProtocol.h"

#pragma comment(lib, "EngineLayer.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	ChatLogicEntry* pLogicEntry = new ChatLogicEntry();

	SFASSERT(false != SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SFJsonProtocol>));
	SFASSERT(false != SFEngine::GetInstance()->Start());

	google::FlushLogFiles(google::GLOG_INFO);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}