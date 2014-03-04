#include "StdAfx.h"
#include "SFStackWalker.h"
#include "SFExceptionHandlerUtil.h"
#include <stdio.h>
#include <fstream>

using namespace  std;

SFStackWalker::SFStackWalker(void)
{
}

void SFStackWalker::SetLogFilePath(void)
{
	GetOutofMemoryLogPath(m_szDumpPath);
}

SFStackWalker::~SFStackWalker(void)
{
}

void SFStackWalker::OnOutput( LPCSTR szText )
{
	wofstream outFile(m_szDumpPath,ios::app);
	if(!outFile.good())
	{
		return;
	}

	outFile<<szText<<endl;
}