////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2002.03.19
// 설  명 :
//

/////////////////////////////////////////////////////////////////////////////////////////
//
#include "AllOcfClientSys.h"
#include "AllOcfClient.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
unsigned int
__stdcall g_fThreadFunc(LPVOID pcComponent)
{
	CThreadClient *pcClass = (CThreadClient*)pcComponent;

	pcClass->SetThreadStatus(THREADSTATUS_STARTING);

#ifdef _DEBUG
	try
	{
#endif
		pcClass->SetThreadExitStatus(THREADEXITSTATUS_NORMAL);
		pcClass->SetEndFlag(TRUE);

		if (FALSE == pcClass->Init())
		{
			pcClass->Uninit();
			pcClass->SetSync();

			pcClass->SetThreadStatus(THREADSTATUS_ZOMBIE);
			return -1;
		}

		pcClass->SetEndFlag(FALSE);
		pcClass->SetSync();

		pcClass->SetThreadStatus(THREADSTATUS_RUNNING);

		if (FALSE == pcClass->Do())
		{
			pcClass->SetThreadStatus(THREADSTATUS_ENDING);

			pcClass->Uninit();
			pcClass->SetEndFlag(TRUE);
			pcClass->SetSync();
		}
		else
		{
			pcClass->SetThreadStatus(THREADSTATUS_ENDING);

			pcClass->SetEndFlag(TRUE);
			pcClass->Uninit();
		}
#ifdef _DEBUG
	}
	catch (...)
	{
		pcClass->SetThreadExitStatus(THREADEXITSTATUS_EXCEPTION);
		pcClass->Uninit();
		pcClass->SetEndFlag(TRUE);
		pcClass->SetSync();
	}
#endif

	pcClass->SetThreadStatus(THREADSTATUS_ZOMBIE);

	_endthreadex(0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
CThreadClient::CThreadClient():
	m_hSync(::CreateEvent(NULL, FALSE, FALSE, NULL))
{
	m_bEndFlag = TRUE;
	m_enumThreadExitStatus = THREADEXITSTATUS_NORMAL;
	m_enumThreadStatus = THREADSTATUS_ZOMBIE;

	m_hThread = NULL;
}

CThreadClient::~CThreadClient()
{
	if (m_hSync)
	{
		CloseHandle(m_hSync);
	}
}

BOOL
CThreadClient::Start()
{
	ResetEvent(m_hSync);

	DWORD dwThreadID;
	m_hThread = reinterpret_cast<HANDLE>(::_beginthreadex(NULL, 0, g_fThreadFunc, (LPVOID)this, 0, (unsigned*)&dwThreadID));
	if (m_hThread)
	{
		WaitSync();

		if (TRUE == m_bEndFlag)
		{
			End();
			return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}

BOOL 
CThreadClient::Init()
{
	if (NULL == m_hSync)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL 
CThreadClient::DoEnd()
{
	if (NULL != m_hThread)
	{
		m_bEndFlag = TRUE;
		return TRUE;
	}

	return FALSE;
}

BOOL
CThreadClient::End(DWORD dwWaitTime)
{

	if (NULL != m_hThread)
	{
		DWORD dwExitCode;
		if (GetExitCodeThread(m_hThread, &dwExitCode) && STILL_ACTIVE == dwExitCode)
		{
			WaitWithMessageLoop(m_hThread, dwWaitTime);

			if (GetExitCodeThread(m_hThread, &dwExitCode) && STILL_ACTIVE == dwExitCode)
			{
				TerminateThread(m_hThread, 0);
				m_enumThreadStatus = THREADSTATUS_ZOMBIE;
			}
		}

		CloseHandle(m_hThread);
		m_hThread = NULL;

		return (TRUE);
	}

	return (FALSE);
}

void 
CThreadClient::WaitSync()
{
	WaitWithMessageLoop(m_hSync);
}

void 
CThreadClient::SetSync()
{
	SetEvent(m_hSync);
}

BOOL 
CThreadClient::WaitWithMessageLoop(HANDLE hEvent, DWORD dwMilliseconds)
{
	while (TRUE)
	{
		DWORD dwReturn = ::MsgWaitForMultipleObjects(1, &hEvent, FALSE, dwMilliseconds, QS_ALLINPUT);

		if (dwReturn == WAIT_OBJECT_0)
		{
			return TRUE;
		}
		else if (dwReturn == WAIT_OBJECT_0 + 1)
		{
			MSG msg;
			while(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				::DispatchMessage(&msg);
			}
		}
		else
		{
			return FALSE;
		}
	}
}

