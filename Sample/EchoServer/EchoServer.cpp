#include "stdafx.h"
#include "EchoLogicEntry.h"
#include "SFJsonProtocol.h"
#include "SFCGSFPacketProtocol.h"
#include "SFClassicDispatcher.h"

#pragma comment(lib, "EngineLayer.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	EchoLogicEntry* pLogicEntry = new EchoLogicEntry();
	
	auto errorCode = SFEngine::GetInstance()->Intialize(pLogicEntry);
	//auto errorCode = SFEngine::GetInstance()->Intialize(pLogicEntry, new SFClassicDispatcher());
	if (errorCode != NET_ERROR_CODE::SUCCESS)
	{	
		return 0;
	}

	SFEngine::GetInstance()->AddPacketProtocol(0, new SFPacketProtocol<SFCGSFPacketProtocol>);
	SFEngine::GetInstance()->Start(0);

	getchar();
	
	SFEngine::GetInstance()->ShutDown();

	return 0;
}