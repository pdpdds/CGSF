// ChatClient.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "SFNetworkEntry.h"
#include "TCPNetworkCallback.h"
#include "BasePacket.h"
#include "SFPacketProtocol.h"
#include "SFJsonProtocol.h"
#include "SFJsonPacket.h"
#include <string>
#include <iostream>


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
		SFNetworkEntry::GetInstance()->TCPSend(&packet);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	TCPNetworkCallback* pCallback = new TCPNetworkCallback();

	SFNetworkEntry::GetInstance()->Initialize(pCallback, new SFPacketProtocol<SFJsonProtocol>);
	SFASSERT(TRUE == SFNetworkEntry::GetInstance()->Run());

	int inputThreadID = ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)EchoInputThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY);

	while(1)
	{
		SFNetworkEntry::GetInstance()->Update();
	
		Sleep(1);
	}

	SFNetworkEntry::GetInstance()->ShutDown();

	return 0;
}