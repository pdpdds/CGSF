#include "stdafx.h"
#include <iostream>
#include "EchoLogicEntry.h"
#include "SFJsonProtocol.h"

#pragma comment(lib, "EngineLayer.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	EchoLogicEntry* pLogicEntry = new EchoLogicEntry();
	
	auto isResult = SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SFJsonProtocol>);
	if (isResult == false)
	{
		std::cout << "엔진 초기화 실패" << std::endl;
		return 0;
	}

	SFEngine::GetInstance()->Start();
	
	google::FlushLogFiles(google::GLOG_INFO);

	getchar();
	
	SFEngine::GetInstance()->ShutDown();

	return 0;
}