#include "StdAfx.h"
#include "SFBugTrap.h"
#include <BugTrap/Bugtrap.h>
#include <tchar.h>
#include <new>
#include "SFExceptionHandlerUtil.h"

using namespace std;

typedef int (ACTIVATEP2P_FUNC(void));

typedef LPTOP_LEVEL_EXCEPTION_FILTER (APIENTRY BUGTRAP_InstallSehFilter(void));
typedef void (APIENTRY BUGTRAP_SetAppName(LPCTSTR pszAppName));
typedef void (APIENTRY BUGTRAP_SetReportFilePath(LPCTSTR pszReportFilePath));
typedef void (APIENTRY BUGTRAP_SetActivityType(BUGTRAP_ACTIVITY eActivityType));
typedef void (APIENTRY BUGTRAP_SetReportFormat(BUGTRAP_REPORTFORMAT eReportFormat));
typedef void (APIENTRY BUGTRAP_SetFlags(DWORD dwFlags));
typedef void (BUGTRAP_InterceptSUEF(HMODULE hModule, BOOL bOverride));

SFBugTrap::SFBugTrap(void)
{
}

SFBugTrap::~SFBugTrap(void)
{
}

BOOL SFBugTrap::Install()
{
	HINSTANCE m_pBugTrapHandle = 0;

	m_pBugTrapHandle = ::LoadLibrary(L"BugTrapU.dll");

	if(m_pBugTrapHandle == 0)
		return FALSE;

	TCHAR szCurDirectory[MAX_PATH] = {0,};
	TCHAR* pEnd = NULL;
	GetModuleFileName(NULL, szCurDirectory, MAX_PATH);
	pEnd = _tcsrchr(szCurDirectory, _T('\\'));

	if(NULL == pEnd)
	{
		return FALSE;
	}

	*pEnd = _T('\0');
	pEnd = pEnd + 1;

	BUGTRAP_InstallSehFilter *pBUGTRAP_InstallSehFilter;
	pBUGTRAP_InstallSehFilter = (BUGTRAP_InstallSehFilter *)::GetProcAddress( m_pBugTrapHandle, "BT_InstallSehFilter" );
	pBUGTRAP_InstallSehFilter();

	BUGTRAP_SetAppName *pBUGTRAP_SetAppName;
	pBUGTRAP_SetAppName = (BUGTRAP_SetAppName *)::GetProcAddress( m_pBugTrapHandle, "BT_SetAppName" );
	pBUGTRAP_SetAppName(pEnd);


	BUGTRAP_SetReportFilePath *pBUGTRAP_SetReportFilePath;
	pBUGTRAP_SetReportFilePath = (BUGTRAP_SetReportFilePath *)::GetProcAddress( m_pBugTrapHandle, "BT_SetReportFilePath" );
	pBUGTRAP_SetReportFilePath(szCurDirectory);


	BUGTRAP_SetActivityType *pBUGTRAP_SetActivityType;
	pBUGTRAP_SetActivityType = (BUGTRAP_SetActivityType *)::GetProcAddress( m_pBugTrapHandle, "BT_SetActivityType" );
	pBUGTRAP_SetActivityType(BTA_SAVEREPORT);


	BUGTRAP_SetReportFormat *pBUGTRAP_SetReportFormat;
	pBUGTRAP_SetReportFormat = (BUGTRAP_SetReportFormat *)::GetProcAddress( m_pBugTrapHandle, "BT_SetReportFormat" );
	pBUGTRAP_SetReportFormat(BTRF_TEXT);


	BUGTRAP_SetFlags *pBUGTRAP_SetFlags;
	pBUGTRAP_SetFlags = (BUGTRAP_SetFlags *)::GetProcAddress( m_pBugTrapHandle, "BT_SetFlags" );
	pBUGTRAP_SetFlags(BTF_DETAILEDMODE);

	BUGTRAP_InterceptSUEF *pBUGTRAP_InterceptSUEF;
	pBUGTRAP_InterceptSUEF = (BUGTRAP_InterceptSUEF *)::GetProcAddress( m_pBugTrapHandle, "BT_InterceptSUEF" );
	pBUGTRAP_InterceptSUEF(GetModuleHandle(NULL), TRUE);


	/*BT_InstallSehFilter();
	BT_SetAppName(pEnd);
	BT_SetReportFilePath(szCurDirectory);
	BT_SetActivityType(BTA_SAVEREPORT);
	BT_SetReportFormat(BTRF_TEXT);
	BT_SetFlags(BTF_DETAILEDMODE);
	BT_InterceptSUEF(GetModuleHandle(NULL), TRUE);*/
	
	std::set_new_handler(New_OutOfMemory_Handler);

	return TRUE;
}