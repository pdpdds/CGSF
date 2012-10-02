#include "StdAfx.h"
#include "SFLogicGateway.h"

SFLogicGateway::SFLogicGateway(void)
{
}

SFLogicGateway::~SFLogicGateway(void)
{
}

BOOL SFLogicGateway::PushPacket( SFCommand* pPacket )
{
	return m_IOCPQueue.Push(pPacket);
}

SFCommand* SFLogicGateway::PopPacket(int WaitTime)
{
	return m_IOCPQueue.Pop(WaitTime);
}