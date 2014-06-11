#include "../Interface/P2PServer.h"

//
#include "AllRelay.h"
#include "conio.h"


SERVICE_STATUS_HANDLE g_hSrv;
DWORD g_NowState;
BOOL g_bPause;
HANDLE g_ExitEvent;

SOCKET dSock;
HANDLE hFile;
int nListenPort;
int nThreadCount;
int nBuffSize;
//
CRelaySlave* apcRelaySlave[0xFF];

P2PSERVER_API int ActivateP2P(unsigned short usListenPort)
{
	int nThreadCount;
	int nBuffSize;
	//
	SOCKET dSock;
	CRelaySlave* apcRelaySlave[0xFF];

	nListenPort = usListenPort;
	nThreadCount = 5;
	nBuffSize = 8096;

	//
	printf("%d %d %d\n", nListenPort, nThreadCount, nBuffSize);

	//
	if (FALSE == InitUdpServerSock(nListenPort, &dSock))
	{
		printf("UDP Bind Error\n");
		return -1;
	}

	//
	if (FALSE == SetSockOptionSendBuffSize(dSock, nBuffSize))
	{
		printf("UDP SetSockOptionSendBuffSize[%d] Error\n", nBuffSize);
		return -2;
	}

	if (FALSE == SetSockOptionRecvBuffSize(dSock, nBuffSize))
	{
		printf("UDP SetSockOptionRecvBuffSize[%d] Error\n", nBuffSize);
		return -3;
	}

	//
	memset(apcRelaySlave, 0x00, sizeof(apcRelaySlave));

	//
	for (int nI = 0; nI < min(nThreadCount, 0xFF); nI++)
	{
		apcRelaySlave[nI] = new CRelaySlave;
		if (NULL == apcRelaySlave[nI])
		{
			printf("Create Slave...Error\n");
			return -4;
		}

		//
		apcRelaySlave[nI]->SetSock(dSock);

		//
		if (FALSE == apcRelaySlave[nI]->Start())
		{
			printf("Create Slave...Error2\n");
			return -5;
		}
	}

	return 0;

}

P2PSERVER_API int DeactivateP2P()
{
	for (int nI = 0; nI < min(nThreadCount, 0xFF); nI++)
	{
		apcRelaySlave[nI]->DoEnd();
		apcRelaySlave[nI]->ThreadEnd();
		delete apcRelaySlave[nI];
	}

	return 0;
}