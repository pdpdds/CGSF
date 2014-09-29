#include "stdafx.h"
#include "RPCLogicEntry.h"
#include "SFCGSFPacketProtocol.h"
#include "RPCService.h"
#include "../Common/RPCProject/testclient.h"

#pragma comment(lib, "EngineLayer.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	RPCLogicEntry* pLogicEntry = new RPCLogicEntry();

	SFEngine::GetInstance()->Intialize(pLogicEntry);
	SFEngine::GetInstance()->AddPacketProtocol(0, new SFPacketProtocol<SFCGSFPacketProtocol>);
	TestInterfaceImpl impl;
	SFEngine::GetInstance()->AddRPCService(new RPCService<TestInterface>(impl));
	SFEngine::GetInstance()->Listen(0);

	google::FlushLogFiles(google::GLOG_INFO);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}