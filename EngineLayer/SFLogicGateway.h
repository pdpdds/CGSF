#pragma once
#include "SFTSSyncQueue.h"
#include "SFIOCPQueue.h"

class BasePacket;

class SFLogicGateway
{
public:
	SFLogicGateway(void);
	virtual ~SFLogicGateway(void);

	bool PushPacket(BasePacket* pPacket);
	BasePacket* PopPacket(int WaitTime = INFINITE);

private:
	SFIOCPQueue<BasePacket> m_IOCPQueue;
	
};

