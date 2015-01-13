#include "stdafx.h"
#include "SFClassicDispatcher.h"
#include "SFEngine.h"

SFClassicDispatcher::SFClassicDispatcher()
{
}

SFClassicDispatcher::~SFClassicDispatcher()
{
}

void SFClassicDispatcher::Dispatch(BasePacket* pPacket)
{
	LogicEntrySingleton::instance()->ProcessPacket(pPacket);

	if (pPacket->GetPacketType() != SFPACKET_DB)
	{
		ReleasePacket(pPacket);
	}	
}

bool SFClassicDispatcher::CreateLogicSystem(ILogicEntry* pLogicEntry)
{
	LogicEntrySingleton::instance()->SetLogic(pLogicEntry);
	return true;
}

bool SFClassicDispatcher::ShutDownLogicSystem()
{
	return true;
}