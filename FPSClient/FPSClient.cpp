// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SFNetworkEntry.h"
#include "TCPCallback.h"
#include "UDPCallback.h"

#pragma comment(lib, "NetworkEngine.lib")
#pragma comment(lib, "NetInterface.lib")

SFNetworkEntry* g_pNetworkEntry = NULL;

int _tmain(int argc, _TCHAR* argv[])
{
	g_pNetworkEntry = new SFNetworkEntry();

	TCPCallback* pCallback = new TCPCallback();
	UDPCallback* pUDPCallback = new UDPCallback(); 

	g_pNetworkEntry->Initialize(pCallback, pUDPCallback);

	g_pNetworkEntry->Run();

	while(1)
	{
		g_pNetworkEntry->Update();
	
		Sleep(1);
	}

	return 0;
}

