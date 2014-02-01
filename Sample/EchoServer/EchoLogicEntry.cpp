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
	if (pPacket->GetPacketType() == SFPacket_Data)
	{
		g_pEngine->SendRequest(pPacket);
	}

	return TRUE;
}

