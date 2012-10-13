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
template<class T> class CMemPool: private CBoList, private CCriticalClient
{
public:
	CMemPool():m_ulCount(0) {}
	virtual ~CMemPool() {}

public:
	virtual T* Allocate();
	virtual BOOL Deallocate(T* pcT);
	virtual BOOL Deallocate(CNode* pcT);

public:
	inline BOOL Init() { return CCritical::IsActive(); }
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

	CCriticalClient::Lock();

	if (TRUE == CBoList::IsEmpty())
	{
		CCriticalClient::Unlock();
		return new T();
	}

	pcNode = static_cast<T*>(CBoList::DelBeginWithLive());

	m_ulCount--;

	CCriticalClient::Unlock();

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

	CCriticalClient::Lock();

	CBoList::AddAtFirst(pcT);

	m_ulCount++;

	CCriticalClient::Unlock();

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
template <class T>
BOOL
CMemPool<T>::Deallocate(CNode* pcT)
{
	if (NULL == static_cast<T*>(pcT))
	{
		return FALSE;
	}

	CCriticalClient::Lock();

	CBoList::AddAtFirst(pcT);

	m_ulCount++;

	CCriticalClient::Unlock();

	return TRUE;
}

#endif

