#include "stdafx.h"
#include "NetworkSystem.h"
#include "SFNetworkEntry.h"
#include "TCPCallback.h"
#include "UDPCallback.h"
#include "CasualGameProtocol.h"
#include "SFPacketProtocol.h"
#include "SFCasualGameDispatcher.h"

NetworkSystem::NetworkSystem(void)
{
}


NetworkSystem::~NetworkSystem(void)
{
}

bool NetworkSystem::Intialize(CasualGameManager* pOwner, IPacketProtocol* pProtocol)
{
	TCPCallback* pCallback = new TCPCallback(pOwner);
	UDPCallback* pUDPCallback = new UDPCallback(); 

	ILogicDispatcher* pDispatcher = new SFCasualGameDispatcher();

	SFNetworkEntry::GetInstance()->Initialize(pCallback, pProtocol, pDispatcher, pUDPCallback);

	if (FALSE == SFNetworkEntry::GetInstance()->Run())
		return FALSE;

	return true;
}

bool NetworkSystem::Update()
{
	SFNetworkEntry::GetInstance()->Update();

	return true;
}

bool NetworkSystem::TCPSend(BasePacket* pPacket )
{
	return SFNetworkEntry::GetInstance()->TCPSend(pPacket);
}

bool NetworkSystem::UDPSend(unsigned char* pMessage, int BufSize )
{
	return SFNetworkEntry::GetInstance()->UDPSend(pMessage, BufSize);
}

bool NetworkSystem::AddPeer(int Serial, int ExternalIP, short ExternalPort, int LocalIP, short LocalPort)
{
	return SFNetworkEntry::GetInstance()->AddPeer(Serial, ExternalIP, ExternalPort, LocalIP, LocalPort);
}

bool NetworkSystem::DeletePeer(int Serial)
{
	return SFNetworkEntry::GetInstance()->DeletePeer(Serial);
}

