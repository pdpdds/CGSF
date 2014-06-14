#include "stdafx.h"
#include "SFClassicDispatcher.h"


SFClassicDispatcher::SFClassicDispatcher()
{
}


SFClassicDispatcher::~SFClassicDispatcher()
{
}

void SFClassicDispatcher::Dispatch(BasePacket* pPacket)
{
	LogicEntrySingleton::instance()->ProcessPacket(pPacket);
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
