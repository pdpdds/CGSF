#pragma once
#include "SFTSSyncQueue.h"
#include "SFIOCPQueue.h"

class IPacketTask;

class SFPacketSendGateway
{
public:
	SFPacketSendGateway(void);
	virtual ~SFPacketSendGateway(void);

	bool PushTask(IPacketTask* pPacketTask);
	IPacketTask* PopTask(int WaitTime = INFINITE);

private:
	SFIOCPQueue<IPacketTask> m_IOCPQueue;
};