#include "stdafx.h"
#include "SFRPCGateway.h"


SFRPCGateway::SFRPCGateway()
{
}


SFRPCGateway::~SFRPCGateway()
{
}

BOOL SFRPCGateway::PushPacket(BasePacket* pPacket)
{
	return m_IOCPQueue.Push(pPacket);
}

BasePacket* SFRPCGateway::PopPacket(int WaitTime)
{
	return m_IOCPQueue.Pop(WaitTime);
}
