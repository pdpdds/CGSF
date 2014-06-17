// ServerListener.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "ServerListenerLogicEntry.h"
#include "SFJsonProtocol.h"

#pragma comment(lib, "EngineLayer.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	ServerListenerLogicEntry* pLogicEntry = new ServerListenerLogicEntry();
	SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SFJsonProtocol>);

	int listenerId = -1;
	listenerId = SFEngine::GetInstance()->AddListener(nullptr, 10000);
	SFASSERT(listenerId != -1);

	listenerId = SFEngine::GetInstance()->AddListener(nullptr, 10001);
	SFASSERT(listenerId != -1);

	listenerId = SFEngine::GetInstance()->AddListener(nullptr, 10002);
	SFASSERT(listenerId != -1);

	SFEngine::GetInstance()->Start();

	google::FlushLogFiles(google::GLOG_INFO);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}