#include "StdAfx.h"
#include "SFPacketProtocol.h"


void SendDummy(BasePacket* pPacket)
{
	LogicGatewaySingleton::instance()->PushPacket(pPacket);
}
