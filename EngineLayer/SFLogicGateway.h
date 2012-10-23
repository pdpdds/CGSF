#pragma once
#include "SFTSSyncQueue.h"
#include "SFIOCPQueue.h"
#include "MPSC_Queue.h"

class SFCommand;

class SFLogicGateway
{
public:
	SFLogicGateway(void);
	virtual ~SFLogicGateway(void);

	BOOL PushPacket(SFCommand* pPacket);
	SFCommand* PopPacket(int WaitTime = INFINITE);

private:
	SFIOCPQueue<SFCommand> m_IOCPQueue;
	//MPSCQueue<SFCommand> m_IOCPQueue;
};

