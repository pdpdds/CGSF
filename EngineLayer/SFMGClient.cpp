#include "stdafx.h"
#include "SFMGClient.h"
#include <WinSock2.h>
#include "AsynchIOService.h"
#include "SFMGClientReceiver.h"

#pragma comment(lib, "MyServerLib.lib")

SFMGClient::SFMGClient(void)
{
}


SFMGClient::~SFMGClient(void)
{
}

BOOL SFMGClient::Run()
{
	return TRUE;
}

BOOL SFMGClient::Start( ILogicEntry* pLogic )
{
	WSADATA wsaData;
	WORD wVer = MAKEWORD(2,2);    
	if (WSAStartup(wVer,&wsaData) != NO_ERROR)
		return FALSE;

	if (LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) 
	{
		WSACleanup();
		return FALSE;
	}

	m_pClientReceiver = new SFMGClientReceiver();
	m_pService = new AsynchIOService(m_pClientReceiver, 1024, 1, "MGServer");

	AsynchSocket* pProtoType = AsynchSocket::GetASPrototype();
	m_pService->connectSocket(1, pProtoType->clone(), "127.0.0.1", 3210);
	m_pService->start();

	if(pLogic)
	{
		LogicEntrySingleton::instance()->SetLogic(pLogic);
		ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)BusinessThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2);
		
	}

	return Run();
}

BOOL SFMGClient::End()
{
	

	return TRUE;
}

BOOL SFMGClient::Send(int Serial, USHORT PacketID, char* pMessage, int BufSize )
{
	m_pClientReceiver->Send(Serial, PacketID, pMessage, BufSize );
	return TRUE;
}

BOOL SFMGClient::Send(int Serial, SFPacket* pPacket)
{
	m_pClientReceiver->Send(Serial, pPacket);
	return TRUE;
}