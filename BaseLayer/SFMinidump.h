#pragma once
#include "SFExceptionHandler.h"
#include <process.h>

class SFMinidump : public SFExceptionHandler
{
public:
	SFMinidump(void);
	virtual ~SFMinidump(void);

	BOOL Install() override;

	static LONG WINAPI CreateMiniDump(PEXCEPTION_POINTERS pException);
	static unsigned __stdcall InstallMiniDump(void* pArg);

protected:
	static BOOL ProcessMiniDump(EXCEPTION_POINTERS* pException);

private:
	static HANDLE m_hTheadCrashEvent;
	static BOOL m_bCrash;
	static EXCEPTION_POINTERS m_ExceptionInfo;
	HANDLE m_hThread;
};
