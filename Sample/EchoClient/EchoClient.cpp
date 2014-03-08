// EchoClient.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "SFNetworkEntry.h"
#include "EchoCallback.h"
#include "BasePacket.h"
#include "SFPacketProtocol.h"
#include "SFJsonProtocol.h"
#include "SFJsonPacket.h"
#include "SFCasualGameDispatcher.h"
#include <string>
#include <iostream>
#include "SFBreakPad.h"

SFNetworkEntry* g_pNetworkEntry = NULL;

void EchoInputThread(void* Args)
{
	std::string input;

	while (g_pNetworkEntry->IsConnected())
	{
		std::cin >> input;

		if(input.compare("exit") == 0)
			break;

		if(input.compare("crash") == 0)
		{
			char* pNullPtr = 0;
			*pNullPtr = 'a';
		}

		SFJsonPacket packet;
		packet.GetData().Add("ECHO", input.c_str());
		g_pNetworkEntry->TCPSend(&packet);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	SFBreakPad exceptionHandler;
	exceptionHandler.Install();

	g_pNetworkEntry = new SFNetworkEntry();
	EchoCallback* pCallback = new EchoCallback();

	g_pNetworkEntry->Initialize(pCallback);

	IPacketProtocol* pProtocol = new SFPacketProtocol<SFJsonProtocol>;
	g_pNetworkEntry->SetPacketProtocol(pProtocol);

	ILogicDispatcher* pDispatcher = new SFCasualGameDispatcher();
	g_pNetworkEntry->SetLogicDispatcher(pDispatcher);

	g_pNetworkEntry->Run();

	g_pNetworkEntry->Update();

	int inputThreadID = ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)EchoInputThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY);

	SFASSERT(inputThreadID != -1);

	while(g_pNetworkEntry->IsConnected())
	{
		g_pNetworkEntry->Update();
	
		Sleep(1);
	}

	ACE_Thread_Manager::instance()->wait_grp(inputThreadID);

	g_pNetworkEntry->Finally();
	delete g_pNetworkEntry;

	return 0;
}