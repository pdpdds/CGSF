#pragma once
#include "SFTSSyncQueue.h"
#include "SFIOCPQueue.h"

class BasePacket;

class SFRPCGateway
{
public:
	SFRPCGateway(void);
	virtual ~SFRPCGateway(void);

	BOOL PushPacket(BasePacket* pPacket);
	BasePacket* PopPacket(int WaitTime = INFINITE);

private:
	SFIOCPQueue<BasePacket> m_IOCPQueue;
	//MPSCQueue<SFCommand> m_IOCPQueue;
};