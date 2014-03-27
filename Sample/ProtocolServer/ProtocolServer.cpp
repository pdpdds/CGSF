// ProtocolServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "ProtocolLogicEntry.h"
#include "SFCGSFPacketProtocol.h"
#include "ProtocolCGSFHandler.h"
#include "ProtocolProtobufHandler.h"
#include "ProtocolServerProtocol.h"

#pragma comment(lib, "EngineLayer.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	//ProtocolLogicEntry<ProtocolCGSFHandler>* pLogicEntry = new ProtocolLogicEntry<ProtocolCGSFHandler>();
	//SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SFCGSFPacketProtocol>);

	ProtocolLogicEntry<ProtocolProtobufHandler>* pLogicEntry = new ProtocolLogicEntry<ProtocolProtobufHandler>();
	SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<ProtocolServerProtocol>);

	SFEngine::GetInstance()->Start();

	google::FlushLogFiles(google::GLOG_INFO);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}