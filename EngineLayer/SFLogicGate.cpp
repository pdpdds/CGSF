#include "stdafx.h"
#include "SFLogicGate.h"


SFLogicGate::SFLogicGate()
{
}


SFLogicGate::~SFLogicGate()
{
}

bool SFLogicGate::PushPacket(BasePacket* pPacket)
{
	m_Lock.Lock();
	return m_IOCPQueue.Push(pPacket);
	//return 0 == m_queue.enqueue_tail(pPacket);

}

BasePacket* SFLogicGate::PopPacket(int WaitTime)
{	
	return m_IOCPQueue.Pop(WaitTime);
	//BasePacket* pPacket = NULL;
	//m_queue.dequeue_head(pPacket);

	//return pPacket;
}

