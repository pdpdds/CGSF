////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2006.11.21
// 설  명 :
//

#ifndef __PULOG_H__
#define __PULOG_H__

////////////////////////////////////////////////////////////////////////////////
//
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
//
class CPuLog: public CCriticalClient
{
friend CPuLog& GetPuLogInstance();

private:
	CPuLog();

public:
	virtual ~CPuLog();

public:
	BOOL Init();
	void Uninit();
	void Logging(const char* pszFormat, ...);

private:
	FILE* m_pfpFile;
};

//
extern CPuLog& GetPuLogInstance();

#endif
