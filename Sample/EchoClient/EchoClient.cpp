// EchoClient.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "EchoCallback.h"
#include "SFNetworkEntry.h"
#include "SFPacketProtocol.h"
#include "SFJsonProtocol.h"
#include "SFJsonPacket.h"
#include <string>
#include <iostream>
#include "json\elements.h"

#pragma comment(lib, "EngineLayer.lib")

#define ECHO_PACKET_ID 1000

void EchoInputThread(void* Args)
{
	std::string input;

	while (SFNetworkEntry::GetInstance()->IsConnected())
	{
		std::cin >> input;

		if(input.compare("exit") == 0)
			break;

		SFJsonPacket packet(ECHO_PACKET_ID);
		
//json array write method 1
		json::Array array;
		
		for (int i = 0; i < 30; i++)
		{
			json::UnknownElement element((json::Number)i);
			array.Insert(element);
		}

		JsonArrayNode arrayNode(array);
			
//json array write method 2
		/*
		JsonArrayNode arrayNode();
		for (int i = 0; i < 30; i++)
		{
			JsonObjectNode node;
			node.Add("e", i);
			arrayNode.Insert(node);
		}*/	

		packet.GetData().Add("ECHO", input.c_str());
		packet.GetData().Add("ArrayData", &arrayNode);

		SFNetworkEntry::GetInstance()->TCPSend(&packet);
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
	EchoCallback* pCallback = new EchoCallback();

	SFNetworkEntry::GetInstance()->Initialize(pCallback, new SFPacketProtocol<SFJsonProtocol>);
	SFNetworkEntry::GetInstance()->Run();

	ProcessInput();

	SFNetworkEntry::GetInstance()->ShutDown();

	return 0;
}