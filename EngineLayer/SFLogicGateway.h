#pragma once
#include "SFTSSyncQueue.h"
#include "SFIOCPQueue.h"
#include "MPSC_Queue.h"

class BasePacket;

class SFLogicGateway
{
public:
	SFLogicGateway(void);
	virtual ~SFLogicGateway(void);

	BOOL PushPacket(BasePacket* pPacket);
	BasePacket* PopPacket(int WaitTime = INFINITE);

private:
	SFIOCPQueue<BasePacket> m_IOCPQueue;
	//MPSCQueue<SFCommand> m_IOCPQueue;
};

