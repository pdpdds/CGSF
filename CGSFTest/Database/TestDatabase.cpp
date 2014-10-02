// TestDatabase.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "SFDatabaseProxy.h"
#include "SFDatabaseProxyLocal.h"
#include "SFDatabaseProxyImpl.h"
#include "SFFastDBAdaptorImpl.h"
#include <iostream>
#include "SFJsonPacket.h"
#include "QueryIdentifier.h"
#include "SFMSSQLAdaptorImpl.h"

void FastDBQueryThread(void* Args);
void MSSQLThread(void* Args);

void testMSSQL();
void testFastDB();

int _tmain(int argc, _TCHAR* argv[])
{
	       
	ACE::init();

	void testMSSQL();

	ACE::fini();

	return 0;
}

void testMSSQL()
{
	DBModuleParams params;	
	params.threadSafe = false;

	SFDatabaseProxy* pProxyLocal = new SFDatabaseProxyLocal<SFMSSQLAdaptorImpl>();
	SFDatabaseProxy* pDatabaseProxy = new SFDatabaseProxyImpl(pProxyLocal);

	pDatabaseProxy->Initialize(params);
	
	ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)MSSQLThread, pDatabaseProxy, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2);

	ACE_Thread_Manager::instance()->wait();
}

void MSSQLThread(void* Args)
{
	SFDatabaseProxy* pDatabaseProxy = (SFDatabaseProxy*)Args;

	std::cout << "Input User Name " << std::endl;

	std::string input;

	while (1)
	{
		std::cin >> input;
		SFMessage* pMessage = SFDatabase::GetInitMessage(DBMSG_LOADUSER, 1000);
		*pMessage << (char*)input.c_str();

		pDatabaseProxy->SendDBRequest(pMessage);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void testFastDB()
{
	DBModuleParams params;
	params.threadSafe = true;

	SFDatabaseProxy* pProxyLocal = new SFDatabaseProxyLocal<SFFastDBAdaptorImpl>();
	SFDatabaseProxy* pDatabaseProxy = new SFDatabaseProxyImpl(pProxyLocal);

	pDatabaseProxy->Initialize(params);

	ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)FastDBQueryThread, pDatabaseProxy, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2);

	ACE_Thread_Manager::instance()->wait();
}

void FastDBQueryThread(void* Args)
{
	SFDatabaseProxy* pDatabaseProxy = (SFDatabaseProxy*)Args;

	std::cout << "Input book name " << std::endl;

	std::string input;

	while (1)
	{
		std::cin >> input;
		SFMessage* pMessage = SFDatabase::GetInitMessage(DBMSG_BOOKINFO, 1000);
		*pMessage << (char*)input.c_str();

		pDatabaseProxy->SendDBRequest(pMessage);
	}
}