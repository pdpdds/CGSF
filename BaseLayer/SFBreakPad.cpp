#include "StdAfx.h"
#include "SFBreakPad.h"
#include "tchar.h"
#include <process.h>
#include "SFExceptionHandlerUtil.h"
#include <BreakPad/exception_handler.h>

using namespace google_breakpad;

#pragma comment(lib, "exception_handler.lib")
#pragma comment(lib, "crash_generation_client.lib")
#pragma comment(lib, "common.lib")

static bool BreakPadHandlerCallBack(const wchar_t* dump_path,
									const wchar_t* minidump_id,
									void* context,
									EXCEPTION_POINTERS* exinfo,
									MDRawAssertionInfo* assertion,
									bool succeeded)
{
	UNREFERENCED_PARAMETER(assertion);
	UNREFERENCED_PARAMETER(exinfo);
	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(dump_path);

	printf("%s is dumped\n", minidump_id);

	return succeeded;
}

SFBreakPad::SFBreakPad(void)
: m_pBreakPadHandler(NULL)
{
}

SFBreakPad::~SFBreakPad(void)
{
	if(m_pBreakPadHandler)
		delete m_pBreakPadHandler;
}

BOOL SFBreakPad::Install()
{
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

	m_pBreakPadHandler = new ExceptionHandler(szCurDirectory, NULL, BreakPadHandlerCallBack, NULL, true);

	PreventSetUnhandledExceptionFilter();
	std::set_new_handler(New_OutOfMemory_Handler);

	return true;
}