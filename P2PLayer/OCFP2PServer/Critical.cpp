////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2002.03.27
// 설  명 :
//

/////////////////////////////////////////////////////////////////////////////////////////
//
#include "AllOcfCliSys.h"
#include "AllOcfCli.h"

/////////////////////////////////////////////////////////////////////////////////////////
// 
CCritical::CCritical()
{
	try
	{
		InitializeCriticalSection(&m_stCriticalSection);
		m_bInitFlag = TRUE;
	}
	catch (...)
	{
		m_bInitFlag = FALSE;
	}
}

CCritical::~CCritical()
{
	if (TRUE == m_bInitFlag)
	{
		DeleteCriticalSection(&m_stCriticalSection);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// 
CSmartLock::CSmartLock(CCritical* pcLock):
	m_pcLock(pcLock)
{
	if (m_pcLock)
	{
		m_pcLock->Lock();
	}
}

CSmartLock::CSmartLock(CCritical& cLock):
	m_pcLock(&cLock)
{
	m_pcLock->Lock();
}

CSmartLock::~CSmartLock()
{
	if (m_pcLock)
	{
		m_pcLock->Unlock();
	}
}
