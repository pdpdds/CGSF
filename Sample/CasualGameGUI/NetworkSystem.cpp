#include "stdafx.h"
#include "NetworkSystem.h"
#include "SFNetworkEntry.h"
#include "TCPCallback.h"
#include "UDPCallback.h"
#include "CasualGameProtocol.h"
#include "SFPacketProtocol.h"
#include "SFCasualGameDispatcher.h"

SFNetworkEntry* g_pNetworkEntry = NULL;

NetworkSystem::NetworkSystem(void)
{
}


NetworkSystem::~NetworkSystem(void)
{
}

bool NetworkSystem::Intialize(CasualGameManager* pOwner, IPacketProtocol* pProtocol)
{
	g_pNetworkEntry = new SFNetworkEntry();

	TCPCallback* pCallback = new TCPCallback(pOwner);
	UDPCallback* pUDPCallback = new UDPCallback(); 

	g_pNetworkEntry->Initialize(pCallback, pUDPCallback);

	g_pNetworkEntry->SetPacketProtocol(pProtocol);

	ILogicDispatcher* pDispatcher = new SFCasualGameDispatcher();
	g_pNetworkEntry->SetLogicDispatcher(pDispatcher);

	if(FALSE == g_pNetworkEntry->Run())
		return FALSE;

	return true;
}

bool NetworkSystem::Update()
{
	g_pNetworkEntry->Update();

	return true;
}

bool NetworkSystem::TCPSend(BasePacket* pPacket )
{
	return g_pNetworkEntry->TCPSend(pPacket);
}

bool NetworkSystem::UDPSend(unsigned char* pMessage, int BufSize )
{
	return g_pNetworkEntry->UDPSend(pMessage, BufSize);
}

bool NetworkSystem::AddPeer(int Serial, int ExternalIP, short ExternalPort, int LocalIP, short LocalPort)
{
	return g_pNetworkEntry->AddPeer(Serial, ExternalIP, ExternalPort, LocalIP, LocalPort);
}

bool NetworkSystem::DeletePeer(int Serial)
{
	return g_pNetworkEntry->DeletePeer(Serial);
}

