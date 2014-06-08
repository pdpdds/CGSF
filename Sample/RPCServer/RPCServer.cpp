#include "stdafx.h"
#include "RPCLogicEntry.h"
#include "SFCGSFPacketProtocol.h"
#include "RPCManager.h"
#include "../Common/RPCProject/testclient.h"

#pragma comment(lib, "EngineLayer.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	RPCLogicEntry* pLogicEntry = new RPCLogicEntry();

	SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SFCGSFPacketProtocol>);
	TestInterfaceImpl impl;
	SFEngine::GetInstance()->RegisterRPCManager(new RPCManager<TestInterface>(impl));
	SFEngine::GetInstance()->Start();

	google::FlushLogFiles(google::GLOG_INFO);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}