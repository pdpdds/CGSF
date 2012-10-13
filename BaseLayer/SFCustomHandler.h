#pragma once
#include "SFExceptionHandler.h"
#include <process.h>

class SFCustomHandler: public SFExceptionHandler
{
public:
	SFCustomHandler(void);
	virtual ~SFCustomHandler(void);

	BOOL Install() override;

protected:
	static unsigned __stdcall ProcessException(void* pArg);
	static LONG WINAPI CustomExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo);
private:
};
