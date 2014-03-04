////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2006.10.19
// 설  명 :
//

#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

////////////////////////////////////////////////////////////////////////////////
//
template<class T> class CMemPool: private CBoList, private CCritical
{
public:
	CMemPool():m_ulCount(0) {}
	virtual ~CMemPool() {}

public:
	virtual T* Allocate();
	virtual BOOL Deallocate(T* pcT);
	virtual BOOL Deallocate(CNode* pcT);

public:
	inline BOOL Init();

public:
	inline ULONG GetCount() { return m_ulCount; }

private:
	ULONG m_ulCount;
};

////////////////////////////////////////////////////////////////////////////////
//
template <class T>
T* 
CMemPool<T>::Allocate()
{
	T* pcNode;

	CCritical::Lock();

	if (TRUE == CBoList::IsEmpty())
	{
		CCritical::Unlock();
		return new T();
	}

	pcNode = dynamic_cast<T*>(CBoList::DelBeginWithLive());

	m_ulCount--;

	CCritical::Unlock();

	return pcNode;
}

////////////////////////////////////////////////////////////////////////////////
//
template <class T>
BOOL
CMemPool<T>::Deallocate(T* pcT)
{
	if (NULL == pcT)
	{
		return FALSE;
	}

	CCritical::Lock();

	CBoList::AddAtFirst(pcT);

	m_ulCount++;

	CCritical::Unlock();

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
template <class T>
BOOL
CMemPool<T>::Deallocate(CNode* pcT)
{
	if (NULL == dynamic_cast<T*>(pcT))
	{
		return FALSE;
	}

	CCritical::Lock();

	CBoList::AddAtFirst(pcT);

	m_ulCount++;

	CCritical::Unlock();

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
template <class T>
BOOL
CMemPool<T>::Init() 
{
	return CCritical::AllocateMutex();
}

#endif

