#include "StdAfx.h"
#include "SFCustomHandler.h"
#include "tchar.h"
#include "crtdbg.h"
#include "stdlib.h"
#include "SFExceptionHandlerUtil.h"
#include <new>
#include <fstream>

using namespace std;

void InvalidParameterHandler (PCTSTR expression,
							  PCTSTR function,
							  PCTSTR file,
							  unsigned int line,
							  uintptr_t pReserved)
{
	_tprintf(_T("function %s\n"), function);
	_tprintf(_T("File %s Line %d\n"), file, line);
	_tprintf(_T("expression %s\n"), expression);

	RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
}

void PureFunctionCallHandler()
{
	RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
}

BOOL GetLogicalAddress(PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD &offset)
{
	MEMORY_BASIC_INFORMATION mbi;

	if(!VirtualQuery(addr, &mbi, sizeof(mbi))) 
		return FALSE;

	DWORD hMod = (DWORD)mbi.AllocationBase;

	if(!GetModuleFileName((HMODULE)hMod, szModule, len))
		return FALSE;

	PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;
	PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHdr);

	DWORD rva = (DWORD)addr - hMod; //Relative Virtual Address is Offset from Module Load Address.

	for(unsigned i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++, pSection++)
	{
		DWORD dwSectionStart = pSection->VirtualAddress;
		DWORD dwSectionEnd = dwSectionStart + max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

		if (rva >= dwSectionStart && rva <= dwSectionEnd)
		{
			section = i + 1;
			offset = rva - dwSectionStart;
			return TRUE;
		}
	}

	return FALSE;
}

SFCustomHandler::SFCustomHandler(void)
{
}

SFCustomHandler::~SFCustomHandler(void)
{
}

BOOL SFCustomHandler::Install()
{
	_CrtSetReportMode(_CRT_ASSERT, 0);

	_invalid_parameter_handler newHandler = InvalidParameterHandler;
	_invalid_parameter_handler oldHandler = _set_invalid_parameter_handler(newHandler);

	_set_purecall_handler(PureFunctionCallHandler);

	SetUnhandledExceptionFilter(CustomExceptionFilter);

	PreventSetUnhandledExceptionFilter();
	std::set_new_handler(New_OutOfMemory_Handler);

	return TRUE;
}

unsigned __stdcall SFCustomHandler::ProcessException(void* pArg)
{
	PEXCEPTION_POINTERS pExceptionInfo = (PEXCEPTION_POINTERS)(pArg);
	TCHAR szCurrentPath[MAX_PATH] = {0,};
	GetUserDefinedLogPath(szCurrentPath);

	wofstream outFile(szCurrentPath, ios::app);

	if(!outFile.good())
	{
		return NULL;
	}

	TCHAR szStr[2000];
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	_stprintf_s(szStr, TEXT("[%d/%d/%d %d:%d:%d] UnHandled Exception Detected!"), sysTime.wYear,
		sysTime.wMonth,
		sysTime.wDay,
		sysTime.wHour,
		sysTime.wMinute,
		sysTime.wSecond);

	outFile << szStr << endl;

	PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

	_stprintf_s(szStr, TEXT("Exception Code : %08X"), pExceptionRecord->ExceptionCode);

	PVOID addr = pExceptionRecord->ExceptionAddress;

	TCHAR szModule[MAX_PATH];
	DWORD dwLength = sizeof(szModule);
	DWORD dwSection = 0;
	DWORD dwOffset = 0;

	MEMORY_BASIC_INFORMATION mbi;

	if(!VirtualQuery(addr, &mbi, sizeof(mbi))) 
		return NULL;

	DWORD hMod = (DWORD)mbi.AllocationBase;

	if(!GetModuleFileName((HMODULE)hMod, szModule, dwLength))
		return NULL;

	PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;
	PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHdr);

	DWORD rva = 0;
	rva = (DWORD)addr ? (DWORD)addr - hMod : rva;

	for(unsigned i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++, pSection++)
	{
		DWORD dwSectionStart = pSection->VirtualAddress;
		DWORD dwSectionEnd = dwSectionStart + max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

		if (rva >= dwSectionStart && rva <= dwSectionEnd)
		{
			dwSection = i + 1;
			dwOffset = rva ? rva - dwSectionStart : dwOffset;
			break;
		}
	}

	_stprintf_s(szStr, TEXT("Fault Address : %04X:%08X %s"), dwSection, dwOffset, szModule);
	outFile << szStr << endl;

	PCONTEXT pCtx = pExceptionInfo->ContextRecord;
	DWORD pc = pCtx->Eip;

	PDWORD pFrame, pPrevFrame;
	pFrame = (PDWORD)pCtx->Ebp;

	do
	{
		TCHAR szModule[MAX_PATH] = _T("");
		DWORD section = 0, offset = 0;

		if(FALSE == GetLogicalAddress((PVOID)pc, szModule, sizeof(szModule), section, offset))
			break;

		_stprintf_s(szStr, L"%08X %08X %04X:%08X %s", pc, pFrame, section, offset, szModule);

		outFile << szStr << endl;

		pc = pFrame[1];
		pPrevFrame = pFrame;
		pFrame = (PDWORD)pFrame[0];

		if((DWORD)pFrame & 3)
			break;

		if(pFrame <= pPrevFrame)
			break;

		if(IsBadWritePtr(pFrame, sizeof(PVOID) * 2))
			break;

	}while(TRUE);


	return NULL;
}

LONG WINAPI SFCustomHandler::CustomExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{	
	if (EXCEPTION_STACK_OVERFLOW == pExceptionInfo->ExceptionRecord->ExceptionCode)
	{
		MinidumpInfo info;
		info.threadId = ::GetCurrentThreadId();
		info.pException = pExceptionInfo;

		HANDLE hThread = (HANDLE)_beginthreadex(0, NULL, ProcessException, &info, 0, NULL);

		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);		
	}
	else
	{
		return ProcessException(pExceptionInfo);				
	}

	return EXCEPTION_EXECUTE_HANDLER;
}