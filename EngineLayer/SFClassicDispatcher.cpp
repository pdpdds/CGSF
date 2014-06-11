#include "stdafx.h"
#include "SFClassicDispatcher.h"


SFClassicDispatcher::SFClassicDispatcher()
: ILogicDispatcher(false)
{
}


SFClassicDispatcher::~SFClassicDispatcher()
{
}

//로직게이트웨이 큐에 패킷을 큐잉한다.
void SFClassicDispatcher::Dispatch(BasePacket* pPacket)
{
	LogicEntrySingleton::instance()->ProcessPacket(pPacket);
}

void SFClassicDispatcher::Finally()
{

}