#pragma once
#include "SFTSSyncQueue.h"
#include "SFLock.h"
#include <deque>

template <typename T>
class SFLockDeque : public SFTSSyncQueue<T>
{
	typedef std::deque<T*> LockDeque;

public:
	SFLockDeque(void)
	{
		Initialize();
	}
	virtual ~SFLockDeque(void)
	{
		Finally();
	}

	virtual BOOL Push(T* pMsg) 
	{
		SFLockHelper LockHelper(&m_Lock);
		m_Deque.push_back(pMsg);

		return TRUE;
	}

	virtual T* Pop() 
	{
		SFLockHelper LockHelper(&m_Lock);
		if(!m_Deque.empty())
		{
			T* pMsg = m_Deque.front();
			m_Deque.pop_front();
			return pMsg;
		}

		return NULL;
	}

protected:
	virtual BOOL Initialize(){return TRUE;}
	virtual BOOL Finally(){return TRUE;}


private:
	SFLock m_Lock;
	LockDeque m_Deque;
};
