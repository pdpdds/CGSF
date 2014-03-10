#include "stdafx.h"
#include "EchoLogicEntry.h"
#include "SFJsonProtocol.h"

#pragma comment(lib, "EngineLayer.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	EchoLogicEntry* pLogicEntry = new EchoLogicEntry();
	
	SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SFJsonProtocol>);
	SFEngine::GetInstance()->Start();
	
	google::FlushLogFiles(google::GLOG_INFO);

	getchar();
	
	SFEngine::GetInstance()->ShutDown();

	return 0;
}