#pragma once

template <typename T>
class SFIOCPQueue// : public SFTSSyncQueue<T>
{
public:
	SFIOCPQueue(void)
	{
		Initialize();
	}

	virtual ~SFIOCPQueue(void)
	{
		Finally();
	}

	//virtual
		BOOL Push(T* pMsg) 
	{
		::PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)pMsg, NULL);
		return TRUE;
	}

/////////////////////////////////////////////////////////////////////////////////////////////
	//virtual 
		T* Pop() 
	{
		ULONG_PTR pCompletionKey = 0;
		DWORD NumberOfBytesTransferred = 0;
		LPOVERLAPPED pOverlapped = NULL;

		SFASSERT( FALSE != ::GetQueuedCompletionStatus(m_hIOCP, &NumberOfBytesTransferred, &pCompletionKey, &pOverlapped, INFINITE));

		return (T*)(pCompletionKey);
	}

	T* Pop(int Wait = INFINITE) 
	{
		ULONG_PTR pCompletionKey = 0;
		DWORD NumberOfBytesTransferred = 0;
		LPOVERLAPPED pOverlapped = NULL;

		//SFASSERT( FALSE != ::GetQueuedCompletionStatus(m_hIOCP, &NumberOfBytesTransferred, &pCompletionKey, &pOverlapped, Wait));

		if(FALSE != ::GetQueuedCompletionStatus(m_hIOCP, &NumberOfBytesTransferred, &pCompletionKey, &pOverlapped, Wait))
			return (T*)(pCompletionKey);

		return NULL;
	}

protected:
	//virtual
		BOOL Initialize()
	{
		m_hIOCP = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

		return m_hIOCP != NULL;
	}

	//virtual 
		BOOL Finally()
	{
		CloseHandle(m_hIOCP);
		return TRUE;
	}

private:
	HANDLE m_hIOCP;
};