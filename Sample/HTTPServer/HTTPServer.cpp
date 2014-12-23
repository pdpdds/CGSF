// HTTPServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "HTTPLogicEntry.h"
#include "SFHTTPProtocol.h"

#pragma comment(lib, "EngineLayer.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	HTTPLogicEntry* pLogicEntry = new HTTPLogicEntry();

	auto errorCode = SFEngine::GetInstance()->Intialize(pLogicEntry);
	if (errorCode != NET_ERROR_CODE::SUCCESS)
	{
		return 0;
	}

	SFEngine::GetInstance()->AddPacketProtocol(0, new SFPacketProtocol<SFHTTPProtocol>);
	SFEngine::GetInstance()->Start(0);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}
