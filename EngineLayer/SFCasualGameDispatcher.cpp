#include "stdafx.h"
#include "SFCasualGameDispatcher.h"

SFCasualGameDispatcher::SFCasualGameDispatcher(void)
{
}


SFCasualGameDispatcher::~SFCasualGameDispatcher(void)
{
}

void SFCasualGameDispatcher::Dispatch(BasePacket* pPacket)
{
	LogicGatewaySingleton::instance()->PushPacket(pPacket);
}


