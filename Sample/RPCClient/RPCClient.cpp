// RPCClient.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "RPCCallback.h"
#include "SFNetworkEntry.h"
#include "SFPacketProtocol.h"
#include "SFCGSFPacketProtocol.h"
#include "SFPacket.h"
#include <string>
#include <iostream>
#include "RPCManager.h"
#include "../Common/RPCProject/testclient.h"
#include "proxy.hpp"
#include "RPCClientTransportLayer.h"

#pragma comment(lib, "EngineLayer.lib")

#define ECHO_PACKET_ID 1000

void RPCInputThread(void* Args)
{
	std::string input;

	RPCClientTransportLayer trans;
	Proxy<TestInterface> proxy(trans);

	while (SFNetworkEntry::GetInstance()->IsConnected())
	{
		std::cin >> input;

		if (input.compare("exit") == 0)
			break;

		//SFPacket packet(ECHO_PACKET_ID);

		std::string str("ABCDE");
		string &retStr = proxy.invoke(&TestInterface::echo)(str);
		std::cout << "[Client]: received returned:" << retStr.c_str() << endl;
		std::cout << "[Client]: param1=" << str.c_str() << endl;

		delete & retStr; // reference to locally allocated obj, it's invoker's responsibility to release.
	}
}

void ProcessInput()
{
	int inputThreadID = ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)RPCInputThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY);

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
	RPCCallback* pCallback = new RPCCallback();
	
	SFNetworkEntry::GetInstance()->Initialize(pCallback, new SFPacketProtocol<SFCGSFPacketProtocol>);
	SFNetworkEntry::GetInstance()->Run();

	ProcessInput();

	SFNetworkEntry::GetInstance()->ShutDown();

	return 0;
}