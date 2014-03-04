////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2006.11.08
// 설  명 :
//

////////////////////////////////////////////////////////////////////////////////
//
#include "AccessAll.h"

////////////////////////////////////////////////////////////////////////////////
//
#define PUCFG_THXCOMM_TIMEOUT					(1000)
#define PUCFG_THXCOMM_SESSIONTIMEOUT			(10)

#define PUCFG_ENVVAR_TEMPDIRECTORY				"TEMP"
#define PUCFG_LOG_FILENAME						"pulog.log"
#define PUCFG_LOG_BACKUPFILENAME				"pulog.bak"

//
#define PUCFG_IPHEADER_LEN						20
#define PUCFG_UDPHEADER_LEN						8
#define PUCFG_UDPMAXMSG_LEN						(0xFFFF-((PUCFG_IPHEADER_LEN) + (PUCFG_UDPHEADER_LEN)))

//
#define PUCFG_DEFAULT_SNDBUFF_LEN				65535
#define PUCFG_DEFAULT_RCVBUFF_LEN				65535

//
#define PUCFG_DEFAULT_SERIAL					(0xFFFFFFFF)

//
#define PUCFG_DEFAULT_ECHOTIMEOUT				(10)		// 초 단위...

////////////////////////////////////////////////////////////////////////////////
//
CPuCfg&
GetPuCfgInstance()
{
	static CPuCfg cNlsCfg;

	return cNlsCfg;
}

////////////////////////////////////////////////////////////////////////////////
//
CPuCfg::CPuCfg()
{
	m_bInit = FALSE;

	m_achLogFile[0] = '\0';
	m_achBackupLogFile[0] = '\0';

	m_ulSerial = PUCFG_DEFAULT_SERIAL;
	m_lEchoTimeout = PUCFG_DEFAULT_ECHOTIMEOUT;
}

CPuCfg::~CPuCfg()
{
}

BOOL
CPuCfg::Init()
{
	if (FALSE == CCriticalClient::IsActive())
	{
		return FALSE;
	}

	//
	CSmartLock cLock(static_cast<CCriticalClient*>(this));

	//
	if (TRUE == m_bInit)
	{
		return TRUE;
	}

	//
	char achLogFile[MAX_PATH + 1];

	DWORD dwRet = ::GetEnvironmentVariableA(PUCFG_ENVVAR_TEMPDIRECTORY, achLogFile, sizeof(achLogFile));
	if (0 == dwRet || sizeof(achLogFile) == dwRet)
	{
		return FALSE;
	}

	//
	_snprintf(m_achLogFile, sizeof(m_achLogFile), "%s\\%s", achLogFile, PUCFG_LOG_FILENAME);
	_snprintf(m_achBackupLogFile, sizeof(m_achBackupLogFile), "%s\\%s", achLogFile, PUCFG_LOG_BACKUPFILENAME);

	//
	m_bInit = TRUE;

	return TRUE;
}

void
CPuCfg::Uninit()
{
	CSmartLock cLock(static_cast<CCriticalClient*>(this));

	m_bInit = FALSE;
}

void
CPuCfg::GetLogFileName(char achLogFile[MAX_PATH + 1])
{
	CSmartLock cLock(static_cast<CCriticalClient*>(this));

	strncpy(achLogFile, m_achLogFile, MAX_PATH);
	achLogFile[MAX_PATH] = '\0';
}

void
CPuCfg::GetBackupLogFileName(char achBackupLogFile[MAX_PATH + 1])
{
	CSmartLock cLock(static_cast<CCriticalClient*>(this));

	strncpy(achBackupLogFile, m_achBackupLogFile, MAX_PATH);
	achBackupLogFile[MAX_PATH] = '\0';
}

DWORD
CPuCfg::GetThxCommTimeout()
{
	return PUCFG_THXCOMM_TIMEOUT;
}

DWORD
CPuCfg::GetThxCommSessionTimeout()
{
	return PUCFG_THXCOMM_SESSIONTIMEOUT;
}

DWORD
CPuCfg::GetMaxMsgLength()
{
	return PUCFG_UDPMAXMSG_LEN;
}

DWORD
CPuCfg::GetDefaultSndBuffLength()
{
	return PUCFG_DEFAULT_SNDBUFF_LEN;
}

DWORD
CPuCfg::GetDefaultRcvBuffLength()
{
	return PUCFG_DEFAULT_RCVBUFF_LEN;
}
