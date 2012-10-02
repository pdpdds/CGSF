#pragma once
#include <queue>
#include "SFLock.h"
#include "SFMessage.h"

template<typename T> class SFObjectPool
{
public:
	SFObjectPool(int MaxCount)
	{
		Init(MaxCount);
	}

	SFObjectPool(){}

	virtual ~SFObjectPool(void);

	void Init(int MaxCount);

	T* Alloc();
	BOOL Release(T* pObj);

protected:

private:
	SFLock m_Lock;
	std::queue<T*> m_Queue;
};

template<typename T>
void SFObjectPool<T>::Init( int MaxCount )
{
	SFASSERT(MaxCount > 0);

	for(int i = 0; i < MaxCount; i++)
	{
		T* pObj = new T;
		m_Queue.push(pObj);
	}
}

template<typename T> 
SFObjectPool<T>::~SFObjectPool()
{
	T* pObj = NULL;

	while(!m_Queue.empty())
	{
		pObj = m_Queue.front();
		delete pObj;
		m_Queue.pop();
	}
}

template<typename T> T* SFObjectPool<T>::Alloc()
{
	SFLockHelper Helper(&m_Lock);
	
	T* pObj = NULL;

	if(m_Queue.empty())
	{
		pObj = new T;
		return pObj;
	}

	pObj = m_Queue.front();
	m_Queue.pop();

	return pObj;
}


template<typename T> BOOL SFObjectPool<T>::Release(T* pObj)
{
	if(NULL == pObj)
		return FALSE;

	SFLockHelper Helper(&m_Lock);
	
	m_Queue.push(pObj);

	return TRUE;
}

