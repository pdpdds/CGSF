#pragma once
#include <boost/serialization/singleton.hpp>
#include <ACE/Method_Request.h>
#include <ACE/Task.h>
#include <ACE/OS.h>
#include <ACE/Condition_T.h>

class BasePacket;

class SFLogicGate : public boost::serialization::singleton<SFLogicGate>
{
	friend class boost::serialization::singleton<SFLogicGate>;

public:
	SFLogicGate();
	virtual ~SFLogicGate();

	static SFLogicGate& Instance() {

		return boost::serialization::singleton<SFLogicGate>::get_mutable_instance();
	}

	bool PushPacket(BasePacket* pPacket);
	BasePacket* PopPacket(int WaitTime = INFINITE);
	bool PopAll(std::queue<BasePacket*>& queue);

	SFLock m_Lock;

private:
	
	ACE_Unbounded_Queue<BasePacket*>	m_queue;
	ACE_Thread_Mutex m_Workers_Lock;
	SFIOCPQueue<BasePacket> m_IOCPQueue;
};

