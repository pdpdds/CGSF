// ChatClient.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "SFNetworkEntry.h"
#include "TCPNetworkCallback.h"
#include "BasePacket.h"
#include "SFPacketProtocol.h"
#include "SFJsonProtocol.h"
#include "SFCasualGameDispatcher.h"

SFNetworkEntry* g_pNetworkEntry = NULL;

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

	while(1)
	{
		g_pNetworkEntry->Update();
	
		Sleep(1);
	}

	return 0;
}