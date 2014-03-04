// ChatClient.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "SFNetworkEntry.h"
#include "TCPNetworkCallback.h"
#include "BasePacket.h"
#include "SFPacketProtocol.h"
#include "SFJsonProtocol.h"
#include "SFCasualGameDispatcher.h"
#include "SFJsonPacket.h"
#include <string>
#include <iostream>

SFNetworkEntry* g_pNetworkEntry = NULL;

void EchoInputThread(void* Args)
{

	std::string input;
	while(1)
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
		packet.GetData().Add("PacketId", 1234);
		packet.GetData().Add("chat", input.c_str());
		g_pNetworkEntry->TCPSend(&packet);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	g_pNetworkEntry = new SFNetworkEntry();

	TCPNetworkCallback* pCallback = new TCPNetworkCallback();

	g_pNetworkEntry->Initialize(pCallback);

	IPacketProtocol* pProtocol = new SFPacketProtocol<SFJsonProtocol>;
	g_pNetworkEntry->SetPacketProtocol(pProtocol);

	ILogicDispatcher* pDispatcher = new SFCasualGameDispatcher();
	g_pNetworkEntry->SetLogicDispatcher(pDispatcher);

	g_pNetworkEntry->Run();

	int inputThreadID = ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)EchoInputThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY);

	while(1)
	{
		g_pNetworkEntry->Update();
	
		Sleep(1);
	}

	return 0;
}