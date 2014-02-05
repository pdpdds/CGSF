// TestDatabase.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "SFDatabaseProxy.h"
#include "SFDatabaseProxyLocal.h"
#include "SFDatabaseProxyImpl.h"
#include "SFFastDBAdaptorImpl.h"

int _tmain(int argc, _TCHAR* argv[])
{
	ACE::init();

	SFDatabaseProxy* pProxyLocal = new SFDatabaseProxyLocal<SFFastDBAdaptorImpl>();
	SFDatabaseProxy* pDatabaseProxy = new SFDatabaseProxyImpl(pProxyLocal);

	pDatabaseProxy->Initialize();

	getchar();

	return 0;
}

