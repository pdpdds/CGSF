#pragma once
#include "SFTSSyncQueue.h"
#include "SFIOCPQueue.h"

class BasePacket;

class SFPacketSendGateway
{
public:
	SFPacketSendGateway(void);
	virtual ~SFPacketSendGateway(void);

	BOOL PushPacket(BasePacket* pPacket);
	BasePacket* PopPacket(int WaitTime = INFINITE);

private:
	SFIOCPQueue<BasePacket> m_IOCPQueue;
};