#include "stdafx.h"
#include "RPCLogicEntry.h"
#include "SFJsonPacket.h"

RPCLogicEntry::RPCLogicEntry(void)
{
}


RPCLogicEntry::~RPCLogicEntry(void)
{
}

bool RPCLogicEntry::Initialize()
{
	return true;
}

bool RPCLogicEntry::ProcessPacket(BasePacket* pPacket)
{
	switch (pPacket->GetPacketType())
	{	
	case SFPACKET_DATA:		
		SFEngine::GetInstance()->SendRequest(pPacket);
		break;
	}

	return true;
}

