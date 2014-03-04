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

void DatabaseTest()
{
	//SFMSSQLAdaptorImpl ODBC;

	_DBConnectionInfo Info = {L"ServiceName", L"test", L"ID", L"Password"};

	/*if(TRUE == ODBC.Initialize(&Info))
	{					
		ODBC.OnLoadUser(1, L"jUHANG");
		
	}*/
}

void QueryThread(void* Args)
{
	SFDatabaseProxy* pDatabaseProxy = (SFDatabaseProxy*)Args;

	std::cout << "Input book name " << std::endl;

	std::string input;

	while(1)
	{
		std::cin >> input;
		//SFMessage* pPacket = new SFJsonPacket();
		SFMessage* pMessage = SFDatabase::GetInitMessage(DBMSG_BOOKINFO, 1000);
		*pMessage << (char*)input.c_str();

		pDatabaseProxy->SendDBRequest(pMessage);
//		packet.GetData().Add("ECHO", input.c_str());
//		g_pNetworkEntry->TCPSend(&packet);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	       
	ACE::init();

	//SFDatabaseProxy* pProxyLocal = new SFDatabaseProxyLocal<SFFastDBAdaptorImpl>();
	SFDatabaseProxy* pProxyLocal = new SFDatabaseProxyLocal<SFMSSQLAdaptorImpl>();
	SFDatabaseProxy* pDatabaseProxy = new SFDatabaseProxyImpl(pProxyLocal);

	pDatabaseProxy->Initialize(1);

	ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)QueryThread, pDatabaseProxy, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2);

	ACE_Thread_Manager::instance()->wait();

	ACE::fini();

	return 0;
}

/*ACE::init();
SFDatabaseProxy* pProxyLocal = new SFDatabaseProxyLocal();
SFDatabaseProxy* pProxy = new SFDatabaseProxyImpl(pProxyLocal);
pProxy->Initialize();

SFObjectPool<SFMessage> Pool(100);
for(int i= 0; i < 100; i++)
{
SFMessage* pMessage = Pool.Alloc();
pProxy->SendDBRequest(pMessage);
}

getchar();
ACE::fini();*/