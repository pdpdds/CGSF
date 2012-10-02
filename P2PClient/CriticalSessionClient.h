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
class CCriticalClient
{
public:
	CCriticalClient();
	virtual ~CCriticalClient();

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
	CSmartLock(CCriticalClient* pcLock);
	CSmartLock(CCriticalClient& cLock);
	virtual ~CSmartLock();

private:
	CCriticalClient* m_pcLock;
};

#endif
