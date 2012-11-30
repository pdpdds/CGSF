#pragma once

class SFLock
{
public:
	SFLock(void)
	{
		InitializeCriticalSection(&m_CS);
	}

	virtual ~SFLock(void)
	{
		DeleteCriticalSection(&m_CS);
	}

	void Lock() {EnterCriticalSection(&m_CS);}
	void Unlock() 
	{
		LeaveCriticalSection(&m_CS);
	}

protected:

private:
	CRITICAL_SECTION m_CS;
};

class SFLockHelper
{
public:
	SFLockHelper(SFLock* pCS)
	{
		pCS->Lock();
		m_pCS = pCS;
	}

	virtual ~SFLockHelper(void)
	{
		m_pCS->Unlock();
	}

protected:

private:
	SFLock* m_pCS;
};