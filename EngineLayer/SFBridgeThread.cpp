#include "StdAfx.h"
#include "SFBridgeThread.h"
#include "SFPacket.h"
#include "SFEngine.h"

void PacketSendThread(void* Args)
{
	SFEngine* pEngine = (SFEngine*)Args;
	IPacketProtocol* pPacketProtocol = pEngine->GetPacketProtocol();

	while (1)
	{
		BasePacket* pPacket = PacketSendSingleton::instance()->PopPacket();

		if (SFPACKET_SERVERSHUTDOWN == pPacket->GetPacketType())
			break;

		pEngine->SendRequest(pPacket);
		pPacketProtocol->DisposePacket(pPacket);
	}
}