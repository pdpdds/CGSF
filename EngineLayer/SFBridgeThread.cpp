#include "StdAfx.h"
#include "SFBridgeThread.h"
#include "SFPacket.h"
#include "SFEngine.h"

void PacketSendThread(void* Args)
{
	SFEngine* pEngine = (SFEngine*)Args;
	INetworkEngine* pNetworkEngine = pEngine->GetNetworkEngine();

	while (SFEngine::GetInstance()->ServerTerminated() == FALSE)
	{
		SFPacket* pPacket = (SFPacket*)PacketSendSingleton::instance()->PopPacket();

		if (SFPACKET_SERVERSHUTDOWN == pPacket->GetPacketType())
			break;

		pNetworkEngine->SendInternal(pPacket->GetOwnerSerial(), (char*)pPacket->GetDataBuffer(), pPacket->GetDataSize());
		PacketPoolSingleton::instance()->Release(pPacket);
	}
}