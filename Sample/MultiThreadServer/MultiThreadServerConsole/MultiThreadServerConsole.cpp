// MultiThreadServerConsole.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"



#include "ServerNetwork.h"


int _tmain(int argc, _TCHAR* argv[])
{
	auto pServerNetwork = new ServerLib::ServerNetwork;

	auto initResult = pServerNetwork->Init(4, 16000, 4012, 0);
	if (initResult != NET_ERROR_CODE::SUCCESS)
	{
		return 0;
	}

	if (pServerNetwork->Start() == false)
	{
		return 0;
	}


	getchar();
	pServerNetwork->Stop();
	return 0;
}

