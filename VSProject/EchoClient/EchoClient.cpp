// EchoClient.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "SFNetworkEntry.h"
#include "EchoCallback.h"
#include "BasePacket.h"
#include "SFPacketProtocol.h"
#include "SFJsonProtocol.h"
#include "SFCasualGameDispatcher.h"

SFNetworkEntry* g_pNetworkEntry = NULL;

void EchoInputThread(void* Args)
{
	//while(g_pNetworkEntry->GetTCPNetwork() == FALSE)
	{
		
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	g_pNetworkEntry = new SFNetworkEntry();

	EchoCallback* pCallback = new EchoCallback();

	g_pNetworkEntry->Initialize("CGSFEngine.dll", pCallback);

	IPacketProtocol* pProtocol = new SFPacketProtocol<SFJsonProtocol>;
	g_pNetworkEntry->SetPacketProtocol(pProtocol);

	ILogicDispatcher* pDispatcher = new SFCasualGameDispatcher();
	g_pNetworkEntry->SetLogicDispatcher(pDispatcher);

	g_pNetworkEntry->Run();

	ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)EchoInputThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2);

	while(1)
	{
		g_pNetworkEntry->Update();
	
		Sleep(1);
	}

	return 0;
}