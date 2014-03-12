// ProtocolClient.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "ProtocolCallback.h"
#include "SFNetworkEntry.h"
#include "SFPacketProtocol.h"
#include "SFCGSFPacketProtocol.h"
#include "SFPacket.h"
#include "SFMessage.h"
#include <string>
#include <iostream>

#pragma comment(lib, "EngineLayer.lib")

void EchoInputThread(void* Args)
{
	std::string input;

	while (SFNetworkEntry::GetInstance()->IsConnected())
	{
		std::cin >> input;

		if (input.compare("exit") == 0)
			break;

		if (input.compare("1") == 0)
		{
			//SFMessage message;
			SFPacket packet(PACKET_SAMPLE1);
			

			float speed = 10.0f;
			float move = 100.0f;
			packet << speed << move;

			packet.SetPacketID(PACKET_SAMPLE1);

			SFNetworkEntry::GetInstance()->TCPSend(&packet);
		}
	}
}

void ProcessInput()
{
	int inputThreadID = ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)EchoInputThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY);

	SFASSERT(inputThreadID != -1);

	while (SFNetworkEntry::GetInstance()->IsConnected())
	{
		SFNetworkEntry::GetInstance()->Update();

		Sleep(1);
	}

	ACE_Thread_Manager::instance()->wait_grp(inputThreadID);
}

int _tmain(int argc, _TCHAR* argv[])
{
	ProtocolCallback* pCallback = new ProtocolCallback();

	SFNetworkEntry::GetInstance()->Initialize(pCallback, new SFPacketProtocol<SFCGSFPacketProtocol>);
	SFNetworkEntry::GetInstance()->Run();

	ProcessInput();

	SFNetworkEntry::GetInstance()->ShutDown();

	return 0;
}