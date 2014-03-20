#include "StdAfx.h"
#include "SFPacketProtocol.h"
#include "SFEngine.h"

void SendDummy(BasePacket* pPacket)
{
	SFEngine::GetInstance()->GetLogicDispatcher()->Dispatch(pPacket);
}
