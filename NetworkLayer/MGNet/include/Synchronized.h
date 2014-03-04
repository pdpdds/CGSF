#ifndef Synchronized_H
#define Synchronized_H

#pragma once

#include "variver.h"
#include <windows.h>

	
class Synchronized;
class Synchronizable
{
private:
	virtual void accquire() = 0;
	virtual void accquireTimeOut(DWORD limit) = 0;
	virtual void release() = 0;

	friend class Synchronized;
	
};

class Synchronized
{
	Synchronizable*	m_obj;
public:

	class NullSynchronizableException
	{
	public:
		NullSynchronizableException(){}

		virtual ~NullSynchronizableException() {}
	};

	Synchronized(Synchronizable* obj) : m_obj(obj)
	{
		if( 0 == m_obj )
			throw new NullSynchronizableException();
		m_obj->accquire();
	}

	virtual ~Synchronized()
	{
		if(m_obj)
			m_obj->release();
	}

};

class CriticalSectionLock : public Synchronizable
{
	CRITICAL_SECTION		cs;
	DWORD					owningThread;
	int						nestingLevel;
public:

	CriticalSectionLock() : owningThread(0), nestingLevel(0)
	{
		InitializeCriticalSection( &cs );
	}

	virtual ~CriticalSectionLock()
	{
		DeleteCriticalSection( &cs );
	}

private:

	virtual void accquire()
	{
		if (owningThread != GetCurrentThreadId()) 
		{
			EnterCriticalSection( &cs );
			
			owningThread = GetCurrentThreadId();

			nestingLevel = 0;

		}
		nestingLevel++;			
	}

	virtual void accquireTimeOut(DWORD limit)
	{
		accquire();
		
	}

	virtual void release()
	{
		if (0 == nestingLevel) 
		{
			owningThread = 0;
			return;
		}
		nestingLevel--;
		if (0 == nestingLevel) 
		{
			owningThread = 0;
			LeaveCriticalSection( &cs );
		}			
	}

	friend class Synchronized;

};

class SemaphoreLock : public Synchronizable
{
	HANDLE			semaphore;
	DWORD			owningThread;
	int				nestingLevel;
public:

	SemaphoreLock() : owningThread(0), nestingLevel(0)
	{
		semaphore = CreateSemaphore(0, 1, 1, 0);
	}

	virtual ~SemaphoreLock()
	{
		CloseHandle( semaphore );
	}

private:

	virtual void accquire()
	{
		if (owningThread != GetCurrentThreadId()) 
		{
			WaitForSingleObject(semaphore, INFINITE);

			//에러처리 추가 필요 20050127 강명규

			owningThread = GetCurrentThreadId();

			nestingLevel = 0;

		}

		nestingLevel++;
	}

	virtual void accquireTimeOut(DWORD limit)
	{
	}

	virtual void release()
	{
		if (0 == nestingLevel) 
		{
			owningThread = 0;
			return;
		}
		nestingLevel--;
		if (0 == nestingLevel) 
		{
			owningThread = 0;
			ReleaseSemaphore(semaphore, 1, 0);
		}
	}

	friend class Synchronized;
};

class Waiter
{
private:

	HANDLE				awakeningEvnet;

public:

	Waiter()
	{
		awakeningEvnet = CreateEvent(0, false, false, 0);
	}

	virtual ~Waiter()
	{
		CloseHandle(awakeningEvnet);
	}

	void notify()
	{
		SetEvent(awakeningEvnet);
	}

	void notifyAll()
	{
		PulseEvent(awakeningEvnet);
	}

	void wait()
	{
		WaitForSingleObject(awakeningEvnet, INFINITE);
	}

	void wait(long timeout)
	{
		if (WaitForSingleObject(awakeningEvnet, timeout) == WAIT_ABANDONED) 
		{
			;
		}
	}

};



#endif



