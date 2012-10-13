#pragma once
#include "SFExceptionHandler.h"

class SFBreakPad : public SFExceptionHandler
{
public:
	SFBreakPad(void);
	virtual ~SFBreakPad(void);

	BOOL Install() override;

protected:

private:
	void* m_pBreakPadHandler;
};