#include "stdafx.h"
#include "SFTCPNetwork.h"
#include "SFMacro.h"
//#include "GoogleLog.h"
#include <EngineInterface/INetworkEngine.h>
#include <EngineInterface/INetworkCallback.h>
#include <EngineInterface/IPacketProtocol.h>
#include "SFEngine.h"
#include "SFIni.h"
#include "SFConstant.h"
#include "SFStructure.h"
#include "SFSinglton.h"
#include "SFBridgeThread.h"
#include "ILogicDispatcher.h"

SFTCPNetwork::SFTCPNetwork(void)
{
	lfds611_queue_new(&m_pQueue, 1000);
}


SFTCPNetwork::~SFTCPNetwork(void)
{
	lfds611_queue_delete(m_pQueue, NULL, NULL);

	if (m_pTCPCallBack)
	{
		delete m_pTCPCallBack;
		m_pTCPCallBack = 0;
	}

	if (m_TCPClient)
	{
		m_TCPClient->ShutDown();
		m_TCPClient = 0;
	}
}

BOOL SFTCPNetwork::Initialize(char* szModuleName, INetworkCallback* pTCPCallBack)
{
	m_TCPClient = SFEngine::GetInstance();

	if (FALSE == m_TCPClient->CreateEngine(szModuleName, FALSE))
	{
		return FALSE;
	}

	m_pTCPCallBack = pTCPCallBack;

	return TRUE;
}

BOOL SFTCPNetwork::Start(char* szIP, unsigned short Port)
{
	return m_TCPClient->Start(szIP, Port);
}

BOOL SFTCPNetwork::Update()
{
	while (1)
	{
		BasePacket* pPacket = LogicGatewaySingleton::instance()->PopPacket(0);

		if (pPacket != NULL)
		{
			switch (pPacket->GetPacketType())
			{

			case SFPACKET_DATA:		
				m_pTCPCallBack->HandleNetworkMessage(pPacket);
				m_TCPClient->ReleasePacket(pPacket);
				break;

			case SFPACKET_RPC:
				lfds611_queue_guaranteed_enqueue(m_pQueue, pPacket);
				break;

			case SFPACKET_CONNECT:			
				m_pTCPCallBack->HandleConnect(pPacket->GetOwnerSerial());
				delete pPacket;
				break;
			case  SFPACKET_DISCONNECT:			
				m_pTCPCallBack->HandleDisconnect(pPacket->GetOwnerSerial());
				delete pPacket;
				break;
			}

			
		}
		else
		{
			break;
		}
	}

	return TRUE;
}

BOOL SFTCPNetwork::SendRequest(BasePacket* pPacket)
{
	/*int HeaderSize = sizeof(SFPacketHeader);

	SFPacket PacketSend;

	PacketSend.SetPacketID(PacketID);

	PacketSend.MakePacket((BYTE*)pMessage, BufSize, CGSF_PACKET_OPTION);
	*/

	pPacket->SetOwnerSerial(m_pTCPCallBack->GetSerial());
	return m_TCPClient->SendRequest(pPacket);
}

void SFTCPNetwork::SetLogicDispatcher(ILogicDispatcher* pDispatcher)
{
	m_TCPClient->SetLogicDispathcer(pDispatcher);
}

void SFTCPNetwork::SetPacketProtocol(IPacketProtocol* pProtocol)
{
	m_TCPClient->SetPacketProtocol(pProtocol);
}

bool SFTCPNetwork::IsConnected()
{
	return m_pTCPCallBack->IsConnected();
}


BasePacket* SFTCPNetwork::GetRPCResult()
{
	BasePacket* pPacket = NULL;

	lfds611_queue_dequeue(m_pQueue, (void**)&pPacket);
	
	return pPacket;
}