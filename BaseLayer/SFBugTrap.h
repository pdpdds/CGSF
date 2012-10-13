#pragma once
#include "SFExceptionHandler.h"
#include <process.h>

class SFBugTrap: public SFExceptionHandler
{
public:
	SFBugTrap(void);
	virtual ~SFBugTrap(void);

	BOOL Install() override;
};
