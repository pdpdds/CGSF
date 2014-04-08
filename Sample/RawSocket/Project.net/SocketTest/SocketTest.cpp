// SocketTest.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <openssl/applink.c>
#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include <StdoutLog.h>
#include <SocketHandler.h>
#include <TcpSocket.h>
#include <ListenSocket.h>
#include <Utility.h>
#include <Parse.h>
#include <Socket.h>
#include <iostream>

#ifdef SOCKETS_NAMESPACE
using namespace SOCKETS_NAMESPACE;
#endif

#include "ProtocolHandler.h"
#include "ProtocolSocket.h"

#define PROTOCOL_SERVER_IP  "127.0.0.1"
#define PROTOCOL_SERVER_PORT 25251

int _tmain(int argc, _TCHAR* argv[])
{
	ProtocolHandler h;
	ProtocolSocket cc(h);
	cc.Open(PROTOCOL_SERVER_IP, PROTOCOL_SERVER_PORT);
	// Add after Open
	h.Add(&cc);
	h.Select(1, 0);
	while (h.GetCount())
	{
		h.Select(1, 0);
	}

	return 0;
}




