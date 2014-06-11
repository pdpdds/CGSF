////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2006.11.09
// 설  명 :
//

////////////////////////////////////////////////////////////////////////////////
//
#include "AccessAll.h"

////////////////////////////////////////////////////////////////////////////////
//
CPuLog&
GetPuLogInstance()
{
	static CPuLog CPuLog;

	return CPuLog;
}

////////////////////////////////////////////////////////////////////////////////
//
CPuLog::CPuLog()
{
	m_pfpFile = NULL;
}

CPuLog::~CPuLog()
{
	if (NULL != m_pfpFile)
	{
		fclose(m_pfpFile);
	}
}

BOOL 
CPuLog::Init()
{
	if (FALSE == CCriticalClient::IsActive())
	{
		return FALSE;
	}

	//
	CSmartLock cLock(static_cast<CCriticalClient*>(this));

	//
	if (NULL != m_pfpFile)
	{
		return TRUE;
	}

	//
	char achLogFile[MAX_PATH + 1];
	char achBackupLogFile[MAX_PATH + 1];

	GetPuCfgInstance().GetLogFileName(achLogFile);
	GetPuCfgInstance().GetBackupLogFileName(achBackupLogFile);

	//
	if ('\0' == achLogFile[0] || '\0' == achBackupLogFile[0])
	{
		return FALSE;
	}

	//
	remove(achBackupLogFile);
	rename(achLogFile, achBackupLogFile);

	//
	m_pfpFile = fopen(achLogFile, "a");
	if (NULL == m_pfpFile)
	{
		return FALSE;
	}

	return TRUE;
}

void 
CPuLog::Uninit()
{
	CSmartLock cLock(static_cast<CCriticalClient*>(this));

	//
	if (NULL != m_pfpFile)
	{
		fclose(m_pfpFile);
		m_pfpFile = NULL;
	}
}

void 
CPuLog::Logging(const char* pszFormat, ...)
{
	//
	SYSTEMTIME stSystemTime;

	//
	CSmartLock cLock(static_cast<CCriticalClient*>(this));

	//
	if (NULL == m_pfpFile)
	{
		return;
	}

	//
	va_list vaParameter;
	va_start(vaParameter, pszFormat);

	//
	GetSystemTime(&stSystemTime);

	//
	fprintf(m_pfpFile, "[%04d.%02d.%02d %02d:%02d:%02d.%d] ", 
		stSystemTime.wYear, 
		stSystemTime.wMonth, 
		stSystemTime.wDay,
		stSystemTime.wHour, 
		stSystemTime.wMinute, 
		stSystemTime.wSecond,
		stSystemTime.wMilliseconds);

	vfprintf(m_pfpFile, pszFormat, vaParameter);
	va_end(vaParameter);

	fflush(m_pfpFile);
}

