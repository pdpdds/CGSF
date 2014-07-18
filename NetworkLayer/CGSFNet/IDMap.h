#pragma once
#include <ace/Map_Manager.h>
#include "Queue.h"

template <typename LockStrategy, typename T, int MaxIDCount>
class SFIDMap
{
	typedef ACE_Map_Manager<int, T*, LockStrategy> IDMap;

public:
	SFIDMap(void) :  m_idleIdQueue(0){}
	virtual ~SFIDMap(void){}

	int Register(T* p)
	{				
		unsigned int id = m_idleIdQueue.Pop();
		if (id == INVALID_ID)
			return INVALID_ID;
		m_IDMap.bind(id, p);
		
		return id;
	}

	void UnRegister(int id)
	{		
		T* t = NULL;
		if (-1 != m_IDMap.find(id, t))
		{
			m_IDMap.unbind(id);
			m_idleIdQueue.Push(id);
		}
	}

	T* Get(int id)
	{
		T* t = NULL;
		if (-1 == m_IDMap.find(id, t))
			return NULL;
		return t;
	}

	int Size()
	{
		return (uint32)m_IDMap.current_size();
	}

protected:
	IDMap m_IDMap;
	IDQueue<MaxIDCount> m_idleIdQueue;	
};