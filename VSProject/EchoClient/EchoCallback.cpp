#include "stdafx.h"
#include "EchoCallback.h"
#include "SFEngine.h"
#include "SFJsonPacket.h"

EchoCallback::EchoCallback(void)
{
}


EchoCallback::~EchoCallback(void)
{
}

bool EchoCallback::HandleNetworkMessage(BasePacket* pPacket)
{
	if (pPacket->GetPacketType() == SFPacket_Data)
	{
		
	}

	return TRUE;
}