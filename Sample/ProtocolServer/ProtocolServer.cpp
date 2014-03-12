// ProtocolServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "ProtocolLogicEntry.h"
#include "SFCGSFPacketProtocol.h"

#pragma comment(lib, "EngineLayer.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	ProtocolLogicEntry* pLogicEntry = new ProtocolLogicEntry();

	SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SFCGSFPacketProtocol>);
	SFEngine::GetInstance()->Start();

	google::FlushLogFiles(google::GLOG_INFO);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}