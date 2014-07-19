// ServerListener.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "ServerListenerLogicEntry.h"
#include "SFJsonProtocol.h"

#pragma comment(lib, "EngineLayer.lib")

#define RPC_LISTEN_PORT 10000
#define GAMESERVER_LISTEN_PORT 10001
#define AUTHSERVER_LISTEN_PORT 10002


int _tmain(int argc, _TCHAR* argv[])
{
	ServerListenerLogicEntry* pLogicEntry = new ServerListenerLogicEntry();
	SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SFJsonProtocol>);

	int listenerId = -1;
	listenerId = SFEngine::GetInstance()->AddListener(nullptr, RPC_LISTEN_PORT);
	SFASSERT(listenerId != -1);

	//pLogicEntry->AddListenerCallback(listenerId, pListenerRPCCallback);

	listenerId = SFEngine::GetInstance()->AddListener(nullptr, GAMESERVER_LISTEN_PORT);
	SFASSERT(listenerId != -1);

	//pLogicEntry->AddListenerCallback(listenerId, pListenerGameServerCallback);

	listenerId = SFEngine::GetInstance()->AddListener(nullptr, AUTHSERVER_LISTEN_PORT);
	SFASSERT(listenerId != -1);

	//pLogicEntry->AddListenerCallback(listenerId, pListenerAuthServerCallback);

	if (false == SFEngine::GetInstance()->Start())
	{
		LOG(ERROR) << "Server Start Fail";
		SFEngine::GetInstance()->ShutDown();
		return 0;
	}

	google::FlushLogFiles(google::GLOG_INFO);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}