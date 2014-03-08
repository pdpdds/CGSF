#include "StdAfx.h"
#include "SFBridgeThread.h"
#include "SFPacket.h"
#include "SFDatabase.h"
#include "SFEngine.h"

SFIOCPQueue<int> IOCPQueue;

BOOL gServerEnd = FALSE;

void BusinessThread(void* Args)
{
	SFEngine* pEngine = (SFEngine*)Args;
	while (gServerEnd == FALSE)
	{
		BasePacket* pPacket = LogicGatewaySingleton::instance()->PopPacket();
		LogicEntrySingleton::instance()->ProcessPacket(pPacket);

		switch (pPacket->GetPacketType())
		{
		case SFPACKET_CONNECT:
		case SFPACKET_DISCONNECT:
		case SFPACKET_TIMER:
		case SFPACKET_DATA:
		case SFPACKET_SHOUTER:
			pEngine->ReleasePacket(pPacket);
			break;

		case SFPACKET_DB:
			SFDatabase::RecallDBMsg((SFMessage*)pPacket);
			break;

		case SFPACKET_SERVERSHUTDOWN:
			return;

		default:   
			SFASSERT(0);
		}
	}
}

void PacketSendThread(void* Args)
{
	SFEngine* pEngine = (SFEngine*)Args;
	INetworkEngine* pNetworkEngine = pEngine->GetNetworkEngine();

	while (gServerEnd == FALSE)
	{
		SFPacket* pPacket = (SFPacket*)PacketSendSingleton::instance()->PopPacket();

		if (SFPACKET_SERVERSHUTDOWN == pPacket->GetPacketType())
			break;

		pNetworkEngine->SendInternal(pPacket->GetOwnerSerial(), (char*)pPacket->GetDataBuffer(), pPacket->GetDataSize());
		PacketPoolSingleton::instance()->Release(pPacket);
	}
}