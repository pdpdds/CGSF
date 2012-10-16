#include "stdafx.h"
#include "SFTCPNetwork.h"
#include "SFMacro.h"
#include "GoogleLog.h"
#include "INetworkEngine.h"
#include "SFEngine.h"
#include "SFIni.h"
#include "SFConstant.h"
#include "SFStructure.h"
#include "SFSinglton.h"
#include "INetworkCallback.h"
#include "SFBridgeThread.h"

SFTCPNetwork::SFTCPNetwork(void)
{
}


SFTCPNetwork::~SFTCPNetwork(void)
{
}

BOOL SFTCPNetwork::Initialize(char* szModuleName, INetworkCallback* pTCPCallBack)
{
	m_TCPClient = new SFSYSTEM();
	m_TCPClient->CreateSystem(szModuleName, NULL); 

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
		SFPacket* pPacket = (SFPacket*)LogicGatewaySingleton::instance()->PopPacket(0);

		if(pPacket != NULL)
		{
			if(pPacket->GetPacketType() == SFCommand_Data)
			{
				m_pTCPCallBack->HandleNetworkMessage(pPacket->GetPacketID(), pPacket->GetDataBuffer(), pPacket->GetDataSize());
			}
			else if(pPacket->GetPacketType() == SFCommand_Connect)
			{
				m_pTCPCallBack->HandleConnect(pPacket->GetOwnerSerial());
			}
			else if(pPacket->GetPacketType() == SFCommand_Disconnect)
			{
				m_pTCPCallBack->HandleDisconnect(pPacket->GetOwnerSerial());
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

BOOL SFTCPNetwork::Send(int Serial, USHORT PacketID, char* pMessage, int BufSize )
{
	int HeaderSize = sizeof(SFPacketHeader);

	SFPacket PacketSend;

	PacketSend.SetPacketID(PacketID);

	PacketSend.MakePacket((BYTE*)pMessage, BufSize, CGSF_PACKET_OPTION);

	return m_TCPClient->GetNetworkEngine()->Send(Serial, (char*)PacketSend.GetHeader(), PacketSend.GetHeaderSize() + PacketSend.GetDataSize() );
}