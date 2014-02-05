#include "stdafx.h"
#include "SFTCPNetwork.h"
#include "SFMacro.h"
//#include "GoogleLog.h"
#include "INetworkEngine.h"
#include "SFEngine.h"
#include "SFIni.h"
#include "SFConstant.h"
#include "SFStructure.h"
#include "SFSinglton.h"
#include "INetworkCallback.h"
#include "SFBridgeThread.h"
#include "ILogicDispatcher.h"
#include "IPacketProtocol.h"

SFTCPNetwork::SFTCPNetwork(void)
{
}


SFTCPNetwork::~SFTCPNetwork(void)
{
}

BOOL SFTCPNetwork::Initialize(char* szModuleName, INetworkCallback* pTCPCallBack)
{
	m_TCPClient = new SFEngine(L"CGSF");

	m_TCPClient->CreateEngine(szModuleName, FALSE); 

	m_pTCPCallBack = pTCPCallBack;

	return TRUE;
}

BOOL SFTCPNetwork::Start(char* szIP, unsigned short Port)
{
	return m_TCPClient->Start(szIP, Port);
}

BOOL SFTCPNetwork::Update()
{
	while(1)
	{
		BasePacket* pPacket = LogicGatewaySingleton::instance()->PopPacket(0);

		if(pPacket != NULL)
		{
			if(pPacket->GetPacketType() == SFPacket_Data)
			{
				m_pTCPCallBack->HandleNetworkMessage(pPacket);
			}
			else if(pPacket->GetPacketType() == SFPacket_Connect)
			{
				m_pTCPCallBack->HandleConnect(pPacket->GetOwnerSerial());
			}
			else if(pPacket->GetPacketType() == SFPacket_Disconnect)
			{
				m_pTCPCallBack->HandleDisconnect(pPacket->GetOwnerSerial());
			}

			//PacketPoolSingleton::instance()->Release(pPacket);
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