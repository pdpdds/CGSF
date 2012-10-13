////////////////////////////////////////////////////////////////////////////////
// 서브시스템  :
// 프로그램 ID : InitUdpServerSock.cc
// 기능요약    : UDP Server 구현...
// 작성일      : 2003.11.14
// 작성자      : 이용휴
// 수정일      :
// 수정자      :
// 수정내역    :
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
#include "AllOcfCliSys.h"
#include "AllOcfCli.h"

////////////////////////////////////////////////////////////////////////////////
//
BOOL
InitUdpServerSock(UINT uiPort, SOCKET* pdUdpServerSock)
{
	struct sockaddr_in stAddrIn;
	SOCKET	socketUdp;
	BOOL	_bSockLibFlag;

	*pdUdpServerSock = -1;

	WSADATA wsaData;
	if (!WSAStartup(MAKEWORD(2,0), &wsaData))
	{
		_bSockLibFlag = TRUE;
	}
	else
	{
		_bSockLibFlag = FALSE;
	}

	stAddrIn.sin_family = AF_INET;
	stAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
	stAddrIn.sin_port = htons((short)uiPort);

	socketUdp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET  == socketUdp)
	{
		return FALSE;
	}

	if (-1 == bind(socketUdp, (const struct sockaddr *)&stAddrIn, sizeof(stAddrIn)))
	{
		closesocket(socketUdp);
		return FALSE;
	}

	*pdUdpServerSock = socketUdp;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
void
UninitUdpServerSock(SOCKET dUdpSock)
{
	closesocket(dUdpSock);
}

BOOL
SetSockOptionSendBuffSize(SOCKET nDesc, int nSize)
{
	if (setsockopt(nDesc, SOL_SOCKET, SO_SNDBUF, (char *)&nSize, sizeof(int)) < 0)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL
SetSockOptionRecvBuffSize(SOCKET nDesc, int nSize)
{
	if (setsockopt(nDesc, SOL_SOCKET, SO_RCVBUF, (char *)&nSize, sizeof(int)) < 0)
	{
		return FALSE;
	}

	return TRUE;
}

