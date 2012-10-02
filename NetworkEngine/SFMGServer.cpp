#include "stdafx.h"
#include "SFMGServer.h"
#include <WinSock2.h>
#include "AsynchIOService.h"
#include "SFMGServerReceiver.h"
#include "Acceptor.h"

#pragma comment(lib, "MyServerLib.lib")

SFMGServer::SFMGServer(void)
{
}


SFMGServer::~SFMGServer(void)
{
}

BOOL SFMGServer::Run()
{
	m_pAcceptor->start();

	return TRUE;
}

BOOL SFMGServer::Start( ILogicEntry* pLogic )
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

	if(pLogic != NULL)
	{
		ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)BusinessThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2) == -1;

		LogicEntrySingleton::instance()->SetLogic(pLogic);
	}

	m_pServerReceiver = new SFMGServerReceiver();
	m_pServer = new AsynchIOService(m_pServerReceiver, 1024, 5, "MGServer");
	
	AsynchSocket* pProtoType = AsynchSocket::GetASPrototype();
	m_pAcceptor = new Acceptor(m_pServer, pProtoType, "127.0.0.1", 3210);
	m_pServer->start();

	return Run();
}

BOOL SFMGServer::End()
{
	m_pAcceptor->stop();
	delete m_pAcceptor;

	m_pServer->stop();
	delete m_pServer;

	delete m_pServerReceiver;

	WSACleanup();

	return TRUE;
}

BOOL SFMGServer::Send(int Serial, USHORT PacketID, char* pMessage, int BufSize )
{
	m_pServerReceiver->Send(Serial, PacketID, pMessage, BufSize );
	return TRUE;
}

BOOL SFMGServer::Send(int Serial, SFPacket* pPacket)
{
	m_pServerReceiver->Send(Serial, pPacket);
	return TRUE;
}