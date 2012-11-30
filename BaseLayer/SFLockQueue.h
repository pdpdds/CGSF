#pragma once
#include "SFTSSyncQueue.h"
#include "SFLock.h"
#include <queue>

template <typename T>
class SFLockQueue : public SFTSSyncQueue<T>
{
	typedef std::queue<T*> LockQueue;

public:
	SFLockQueue(void)
	{
		Initialize();
	}
	virtual ~SFLockQueue(void)
	{
		Finally();
	}

	virtual BOOL Push(T* pMsg) 
	{
		SFLockHelper LockHelper(&m_Lock);
		m_Queue.push(pMsg);

		return TRUE;
	}

	virtual T* Pop() 
	{
		SFLockHelper LockHelper(&m_Lock);
		if(!m_Queue.empty())
		{
			T* pMsg = m_Queue.front();
			m_Queue.pop();
			return pMsg;
		}

		return NULL;
	}

	int Size(){return m_Queue.size();}

protected:
	virtual BOOL Initialize(){return TRUE;}
	virtual BOOL Finally(){return TRUE;}


private:
	SFLock m_Lock;
	LockQueue m_Queue;
};
