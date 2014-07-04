#include "stdafx.h"
#include "NetworkSystem.h"
#include "SFNetworkEntry.h"
#include "TCPCallback.h"
#include "UDPCallback.h"
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

bool NetworkSystem::UDPSend(unsigned char* pMessage, int bufSize )
{
	return SFNetworkEntry::GetInstance()->UDPSend(pMessage, bufSize);
}

bool NetworkSystem::AddPeer(int serial, int externalIP, short externalPort, int localIP, short localPort)
{
	return SFNetworkEntry::GetInstance()->AddPeer(serial, externalIP, externalPort, localIP, localPort);
}

bool NetworkSystem::DeletePeer(int serial)
{
	return SFNetworkEntry::GetInstance()->DeletePeer(serial);
}

