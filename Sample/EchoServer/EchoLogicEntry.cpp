#include "stdafx.h"
#include "EchoLogicEntry.h"
#include "SFJsonPacket.h"

EchoLogicEntry::EchoLogicEntry(void)
{
}


EchoLogicEntry::~EchoLogicEntry(void)
{
}

bool EchoLogicEntry::Initialize()
{
	return true;
}

bool EchoLogicEntry::ProcessPacket(BasePacket* pPacket )
{
	switch (pPacket->GetPacketType())
	{	
	case SFPACKET_DATA:		
		SFEngine::GetInstance()->SendRequest(pPacket);
		break;
	}

	return true;
}

