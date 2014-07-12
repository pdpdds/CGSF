#include "stdafx.h"
#include "SFPacketSendGateway.h"


SFPacketSendGateway::SFPacketSendGateway(void)
{
}


SFPacketSendGateway::~SFPacketSendGateway(void)
{
}

bool SFPacketSendGateway::PushTask(IPacketTask* pPacketTask)
{
	return m_IOCPQueue.Push(pPacketTask);
}

IPacketTask* SFPacketSendGateway::PopTask(int WaitTime)
{
	return m_IOCPQueue.Pop(WaitTime);
}
