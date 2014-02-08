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

	SFDatabaseProxy* pProxyLocal = new SFDatabaseProxyLocal<SFFastDBAdaptorImpl>();
	SFDatabaseProxy* pDatabaseProxy = new SFDatabaseProxyImpl(pProxyLocal);

	pDatabaseProxy->Initialize(1);

	ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)QueryThread, pDatabaseProxy, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2);

	ACE_Thread_Manager::instance()->wait();

	ACE::fini();

	return 0;
}