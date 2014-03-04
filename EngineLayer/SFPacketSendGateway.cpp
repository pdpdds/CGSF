#include "stdafx.h"
#include "SFPacketSendGateway.h"


SFPacketSendGateway::SFPacketSendGateway(void)
{
}


SFPacketSendGateway::~SFPacketSendGateway(void)
{
}

BOOL SFPacketSendGateway::PushPacket( BasePacket* pPacket )
{
	return m_IOCPQueue.Push(pPacket);
}

BasePacket* SFPacketSendGateway::PopPacket(int WaitTime)
{
	return m_IOCPQueue.Pop(WaitTime);
}
