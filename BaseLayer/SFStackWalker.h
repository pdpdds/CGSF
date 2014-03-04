#pragma once
#include "StackWalker.h"

class SFStackWalker : public StackWalker
{
public:
	SFStackWalker(void);
	virtual ~SFStackWalker(void);

	virtual void OnOutput(LPCSTR szText) override;
	void SetLogFilePath(void);

protected:

private:
	TCHAR m_szDumpPath[MAX_PATH];
};
