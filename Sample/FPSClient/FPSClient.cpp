// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SFNetworkEntry.h"
#include "TCPCallback.h"
#include "UDPCallback.h"
#include "FPSClientProtocol.h"
#include "SFPacketProtocol.h"
#include "SFCasualGameDispatcher.h"

SFNetworkEntry* g_pNetworkEntry = NULL;

///////////////////////////////////////////////////////////////////////////////////
//이 예제는 TCP가 메인 쓰레드에서 동작하고, UDP는 별도의 쓰레드에서 동작하면서 패킷을 메인 쓰레드로 넘기는 예제입니다.
//이후 TCP도 별도의 쓰레드로 분리해서 TCP/UDP/메인이 별도의 쓰레드로 돌아갈 수 있도록 수정할 것입니다.
///////////////////////////////////////////////////////////////////////////////////

int _tmain(int argc, _TCHAR* argv[])
{
	g_pNetworkEntry = new SFNetworkEntry();

	TCPCallback* pCallback = new TCPCallback();
	UDPCallback* pUDPCallback = new UDPCallback(); 

	g_pNetworkEntry->Initialize("CGSFEngine.dll", pCallback, pUDPCallback);

	IPacketProtocol* pProtocol = new SFPacketProtocol<FPSClientProtocol>;
	g_pNetworkEntry->SetPacketProtocol(pProtocol);

	ILogicDispatcher* pDispatcher = new SFCasualGameDispatcher();
	g_pNetworkEntry->SetLogicDispatcher(pDispatcher);


	g_pNetworkEntry->Run();

	while(1)
	{
		g_pNetworkEntry->Update();
	
		Sleep(1);
	}

	return 0;
}