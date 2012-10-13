#include "StdAfx.h"
#include "SFMinidump.h"
#include "tchar.h"
#include "SFExceptionHandlerUtil.h"
#include <fstream>

using namespace std;

HANDLE SFMinidump::m_hTheadCrashEvent = NULL;
BOOL SFMinidump::m_bCrash = FALSE;
EXCEPTION_POINTERS SFMinidump::m_ExceptionInfo = {0,};


LONG WINAPI SFMinidump::CreateMiniDump(PEXCEPTION_POINTERS pException)
{
	m_bCrash = TRUE;

	m_ExceptionInfo = *pException;

	SetEvent(m_hTheadCrashEvent);
	WaitForSingleObject(m_hTheadCrashEvent, INFINITE);

	return EXCEPTION_EXECUTE_HANDLER;
}

unsigned __stdcall SFMinidump::InstallMiniDump(void* pArg)
{
	SetUnhandledExceptionFilter(CreateMiniDump);

	PreventSetUnhandledExceptionFilter();
	std::set_new_handler(New_OutOfMemory_Handler);

	SFMinidump* pMiniDump = (SFMinidump*)pArg;

	WaitForSingleObject(m_hTheadCrashEvent, INFINITE);
	
	if(m_bCrash == TRUE)
	{
		ProcessMiniDump(&m_ExceptionInfo);
		SetEvent(m_hTheadCrashEvent);
	}

	return 0;
}



SFMinidump::SFMinidump(void)
{
}

SFMinidump::~SFMinidump(void)
{
	SetEvent(m_hTheadCrashEvent);
	WaitForSingleObject(m_hThread, INFINITE);
}

BOOL SFMinidump::Install()
{
	m_hTheadCrashEvent = CreateEvent(NULL, FALSE, FALSE, L"Crash");

	m_hThread = (HANDLE)_beginthreadex(0, NULL, InstallMiniDump, this, 0, NULL);

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////
//My MiniDump Handler
////////////////////////////////////////////////////////////////////////////////////////

BOOL SFMinidump::ProcessMiniDump(EXCEPTION_POINTERS* pException)
{
	LONG retval = EXCEPTION_CONTINUE_SEARCH;
	HWND hParent = NULL;						// find a better value for your app

	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old 
	// (e.g. Windows 2000)
	HMODULE hDll = NULL;
	TCHAR szDbgHelpPath[MAX_PATH];

	if (GetModuleFileName( NULL, szDbgHelpPath, MAX_PATH ))
	{
		TCHAR *pSlash = _tcsrchr( szDbgHelpPath, '\\' );
		if (pSlash)
		{
			_tcscpy_s( pSlash+1,sizeof(TEXT("DBGHELP.DLL")), TEXT("DBGHELP.DLL") );
			hDll = ::LoadLibrary( szDbgHelpPath );
		}

		*pSlash = _T('\0');
	}

	if (hDll==NULL)
	{
		// load any version we can
		hDll = ::LoadLibrary( TEXT("DBGHELP.DLL"));
	}

	LPCTSTR szResult = NULL;


	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
		if (pDump)
		{
			TCHAR szDumpPath[MAX_PATH];
			TCHAR szScratch [MAX_PATH];

			// work out a good place for the dump file
			//if (!GetTempPath( MAX_PATH, szDumpPath ))
			_tcscpy_s( szDumpPath, szDbgHelpPath);

			SYSTEMTIME sysTime;
			GetLocalTime(&sysTime);

			_stprintf_s( szScratch, TEXT("%d%d%d%d%d%d"), sysTime.wYear, 
														  sysTime.wMonth, 
														  sysTime.wDay,
														  sysTime.wHour,
														  sysTime.wMinute,  
														  sysTime.wSecond);

			_tcscat_s( szDumpPath, TEXT("\\MiniDump_"));
			_tcscat_s( szDumpPath, szScratch);
			_tcscat_s( szDumpPath, TEXT(".dmp"));

			// create the file
			HANDLE hFile = ::CreateFile( szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL );

			if (hFile!=INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

				ExInfo.ThreadId = ::GetCurrentThreadId();
				ExInfo.ExceptionPointers = pException;
				ExInfo.ClientPointers = NULL;

				// write the dump
				BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
				if (bOK)
				{
					_stprintf_s( szScratch, TEXT("Saved dump file to '%s'"), szDumpPath );
					szResult = szScratch;
					retval = EXCEPTION_EXECUTE_HANDLER;
				}
				else
				{
					_stprintf_s( szScratch, TEXT("Failed to save dump file to '%s' (error %d)"), szDumpPath, GetLastError() );
					szResult = szScratch;
				}
				::CloseHandle(hFile);
			}
			else
			{
				_stprintf_s( szScratch, TEXT("Failed to create dump file '%s' (error %d)"), szDumpPath, GetLastError() );
				szResult = szScratch;
			}

		}
		else
		{
			szResult = TEXT("DBGHELP.DLL too old");
		}
	}
	else
	{
		szResult = TEXT("DBGHELP.DLL not found");
	}

	return retval;
}