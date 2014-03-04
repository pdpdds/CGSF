#pragma once
#include "DbgHelp.h"

BOOL PreventSetUnhandledExceptionFilter();
BOOL RecoverSetUnhandledExceptionFilter();
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);


typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
										 CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
										 CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
										 CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
										 );

void New_OutOfMemory_Handler();
unsigned __stdcall CreateOutofMemoryLog(void* pArguments);

//////////////////////////////////////////////////////////////
//유틸리티 함수
//////////////////////////////////////////////////////////////
void GetOutofMemoryLogPath(TCHAR* pPathName);
void GetUserDefinedLogPath(TCHAR* pPathName);
void AddOutofMemoryLogHeader(DWORD dwCurrentThreadId);

typedef struct _MinidumpInfo
{
	DWORD threadId;    
	EXCEPTION_POINTERS* pException;
} MinidumpInfo, *PMinidumpInfo;