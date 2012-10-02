#include "StdAfx.h"
#include "tchar.h"
#include "Process.h"
#include "SFExceptionHandlerUtil.h"
#include "SFStackWalker.h"
#include <fstream>

using namespace  std;

char g_pSetUnhandledFilterEntryValue[5] = {0,};

////////////////////////////////////////////////////////////////////////////////////////
//Exception Unhandled Filter가 Overriding되는 것을 막기 위한 루틴
//Kernel32.dll의 SetUnhandledExceptionFilter 함수의 진입점에 5바이트 점프코드를 써서
//MyDummySetUnhandledExceptionFilter가 호출되게 하고 이 함수에서는 어떠한 처리도 수행하지 않음
//결과적으로 CRT 함수 등의 set_invalid_parameter나 pure_function_call을 따로 구현해 줄 필요가 없게 된다.
////////////////////////////////////////////////////////////////////////////////////////
#ifndef _M_IX86
#error "The following code only works for x86!"
#endif
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(
	LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	return NULL;
}

BOOL PreventSetUnhandledExceptionFilter()
{
	HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));

	if (hKernel32  == NULL) return FALSE;

	void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
	if(NULL == pOrgEntry) 
		return FALSE;

	memcpy(g_pSetUnhandledFilterEntryValue, pOrgEntry, 5 * sizeof(char));

	unsigned char newJump[ 100 ];
	DWORD dwOrgEntryAddr = (DWORD) pOrgEntry;

	dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far
	void *pNewFunc = &MyDummySetUnhandledExceptionFilter;
	DWORD dwNewEntryAddr = (DWORD) pNewFunc;

	DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;

	newJump[ 0 ] = 0xE9;  // JMP absolute
	memcpy(&newJump[ 1 ], &dwRelativeAddr, sizeof(pNewFunc));
	SIZE_T bytesWritten;
	BOOL bRet = WriteProcessMemory(GetCurrentProcess(),
		pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);	

	return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////
//Exception Unhandled Filter 함수가 동작하도록 복구하는 루틴
//검증된 모듈을 사용하고 싶고, 검증된 모듈에서 보다 확장된 기능을 추가하고 싶은 경우
//SetUnhandledFilter 함수가 제대로 동작하도록 원래대로 돌릴 필요가 있으며 그때 사용하는 함수
//FS 레지스터를 통해 예외함수 체인에 추가하는 방식으로 구현할 수도 있으나 잘되지 않아서....
////////////////////////////////////////////////////////////////////////////////////////
BOOL RecoverSetUnhandledExceptionFilter()
{
	HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));

	if (hKernel32  == NULL) return FALSE;

	void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
	if(NULL == pOrgEntry) 
		return FALSE;

	if(g_pSetUnhandledFilterEntryValue[0] == 0)
		return FALSE;

	//memcpy(pOrgEntry, g_pSetUnhandledFilterEntryValue, 5 * sizeof(char));
	
	SIZE_T bytesWritten;
	BOOL bRet = WriteProcessMemory(GetCurrentProcess(),
		pOrgEntry, g_pSetUnhandledFilterEntryValue, 5 * sizeof(char), &bytesWritten);

	return bRet;
}

void New_OutOfMemory_Handler()
{
	DWORD dwCurrentThreadId = GetCurrentThreadId();
	
	AddOutofMemoryLogHeader(dwCurrentThreadId);

	HANDLE hCurrentThread = OpenThread(THREAD_GET_CONTEXT, FALSE, dwCurrentThreadId);

	HANDLE hThread = (HANDLE)_beginthreadex(0, NULL, CreateOutofMemoryLog, &hCurrentThread, 0, NULL);

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	
	exit(0);
	//RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
}


void GetOutofMemoryLogPath(TCHAR* pPathName)
{
	TCHAR szCurDirectory[MAX_PATH] = {0,};
	TCHAR* pEnd = NULL;
	GetModuleFileName(NULL, szCurDirectory, MAX_PATH);
	pEnd = _tcsrchr(szCurDirectory, _T('\\'));

	if(NULL == pEnd)
	{
		return;
	}

	*pEnd = _T('\0');
	pEnd = pEnd + 1;


	_tcscpy_s( pPathName, MAX_PATH-1, szCurDirectory);
	_tcscat_s( pPathName,  MAX_PATH-1, TEXT("\\OutofMemoryLog.txt"));
}

void GetUserDefinedLogPath(TCHAR* pPathName)
{
	TCHAR szCurDirectory[MAX_PATH] = {0,};
	TCHAR* pEnd = NULL;
	GetModuleFileName(NULL, szCurDirectory, MAX_PATH);
	pEnd = _tcsrchr(szCurDirectory, _T('\\'));

	if(NULL == pEnd)
	{
		return;
	}

	*pEnd = _T('\0');
	pEnd = pEnd + 1;


	_tcscpy_s( pPathName, MAX_PATH-1, szCurDirectory);
	_tcscat_s( pPathName,  MAX_PATH-1, TEXT("\\UserDefinedLog.txt"));
}

void AddOutofMemoryLogHeader(DWORD dwCurrentThreadId)
{
	wprintf(L"Out of Memory %d\n", dwCurrentThreadId);

	TCHAR szCurrentPath[MAX_PATH] = {0,};
	GetOutofMemoryLogPath(szCurrentPath);

	wofstream outFile(szCurrentPath,ios::app);
	if(!outFile.good())
	{
		return;
	}

	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	outFile << "OccurTime " << sysTime.wYear << '/'<< sysTime.wMonth << '/'
		<< sysTime.wDay << ' ' << sysTime.wHour << ':' << sysTime.wMinute 
		<< ':' << sysTime.wSecond << endl;

	outFile << "Out Of Memory. Thread ID : " << dwCurrentThreadId << endl;
}

//////////////////////////////////////////////////////////////
//유틸리티 함수
//////////////////////////////////////////////////////////////
unsigned __stdcall CreateOutofMemoryLog( void* pArguments )
{
	HANDLE hThread = (*(HANDLE*)pArguments);

	SFStackWalker StackWalker;
	StackWalker.SetLogFilePath();

	StackWalker.ShowCallstack(hThread);
		
	_endthreadex(0);
	return 0;
}