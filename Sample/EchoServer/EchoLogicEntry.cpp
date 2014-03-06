#include "stdafx.h"
#include "EchoLogicEntry.h"
#include "SFEngine.h"
#include "SFJsonPacket.h"

extern SFEngine* g_pEngine;

EchoLogicEntry::EchoLogicEntry(void)
{
}


EchoLogicEntry::~EchoLogicEntry(void)
{
}

BOOL EchoLogicEntry::Initialize()
{
	return TRUE;
}

BOOL EchoLogicEntry::ProcessPacket(BasePacket* pPacket )
{
	if (pPacket->GetPacketType() == SFPACKET_DATA)
	{
		g_pEngine->SendRequest(pPacket);
	}
	else if (pPacket->GetPacketType() == SFPACKET_CONNECT)
	{
		int i = 1;
	}

	return TRUE;
}

