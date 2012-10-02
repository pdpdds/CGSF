#include "StdAfx.h"
#include "SFClient.h"
#include "SFProactorService.h"
#include "SFProactorClient.h"
#include "INetworkCallback.h"
#include "INetworkFramework.h"

SFClient::SFClient(void)
{
}

SFClient::~SFClient(void)
{
	if(m_pNetworkCallback) 
		delete m_pNetworkCallback;
}

BOOL SFClient::Run( INetworkFramework* pFramework, ILogicEntry* pLogic )
{
	if(!pFramework->Initialize(NetworkFramework_CLIENT))
		return FALSE;

	if(pFramework->Start(pLogic) == FALSE)
		return FALSE;

	m_pNetworkFramework = pFramework;

	return TRUE;
}

//게임 로직과 네트워크 처리를 한 쓰레드로 처리하고 싶을 때 사용할 것 
BOOL SFClient::Update()
{
	while(1)
	{
		SFPacket* pPacket = (SFPacket*)LogicGatewaySingleton::instance()->PopPacket(0);

		if(pPacket != NULL)
		{
			if(pPacket->GetPacketType() == SFCommand_Data)
			{
				m_pNetworkCallback->HandleNetworkMessage(pPacket->GetPacketID(), pPacket->GetDataBuffer(), pPacket->GetDataSize());
			}
			else if(pPacket->GetPacketType() == SFCommand_Connect)
			{
				m_pNetworkCallback->HandleConnect();
			}
			else if(pPacket->GetPacketType() == SFCommand_Disconnect)
			{
				m_pNetworkCallback->HandleDisconnect();
			}

			PacketPoolSingleton::instance()->Release(pPacket);
		}
		else
		{
			break;
		}
	}

	return TRUE;
}

bool SFClient::Send( USHORT PacketID, char* pMessage, int BufSize )
{
	SFProactorClient* pClient = (SFProactorClient*)m_pNetworkFramework->GetNetworkAgent();
	SFProactorService* pService = (SFProactorService*)pClient->GetNetworkService();
	pService->Send(PacketID, pMessage, BufSize);

	return TRUE;
}