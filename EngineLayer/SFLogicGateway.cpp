#include "StdAfx.h"
#include "SFLogicGateway.h"

SFLogicGateway::SFLogicGateway(void)
{
}

SFLogicGateway::~SFLogicGateway(void)
{
}

bool SFLogicGateway::PushPacket( BasePacket* pPacket )
{
	return m_IOCPQueue.Push(pPacket);
}

BasePacket* SFLogicGateway::PopPacket(int WaitTime)
{
	return m_IOCPQueue.Pop(WaitTime);
}