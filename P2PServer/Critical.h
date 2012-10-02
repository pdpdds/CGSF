////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2003.03.08
// 설  명 :
//

#ifndef __CRITICALSESSIONCLIENT_H__
#define __CRITICALSESSIONCLIENT_H__

////////////////////////////////////////////////////////////////////////////////
//
class CCritical
{
public:
	CCritical();
	virtual ~CCritical();

public:
	inline void Lock() { EnterCriticalSection(&m_stCriticalSection); }
	inline void Unlock() { LeaveCriticalSection(&m_stCriticalSection); }
	inline BOOL IsActive() { return m_bInitFlag; }

private:
	BOOL m_bInitFlag;
	CRITICAL_SECTION m_stCriticalSection;
};

////////////////////////////////////////////////////////////////////////////////
//
class CSmartLock
{
public:
	CSmartLock(CCritical* pcLock);
	CSmartLock(CCritical& cLock);
	virtual ~CSmartLock();

private:
	CCritical* m_pcLock;
};

#endif
