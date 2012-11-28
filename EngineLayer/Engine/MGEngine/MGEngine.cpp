#include "MGEngine.h"
#include <WinSock2.h>
#include "AsynchIOService.h"
#include "MGServerReceiver.h"
#include "MGClientReceiver.h"
#include "Acceptor.h"
#include "IEngine.h"

#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
#pragma comment(lib, "MyServerLib_d.lib")
#else
#pragma comment(lib, "MyServerLib.lib")
#endif

INetworkEngine * CreateNetworkEngine(bool Server, IEngine* pEngine)
{
	if(Server)
		return new MGServerEngine(pEngine);
	else
		return new MGClientEngine(pEngine);
}

MGServerEngine::MGServerEngine(IEngine* pEngine)
	: INetworkEngine(pEngine)
{
}


MGServerEngine::~MGServerEngine(void)
{
}

bool MGServerEngine::Init()
{
	WSADATA wsaData;
	WORD wVer = MAKEWORD(2,2);    
	if (WSAStartup(wVer,&wsaData) != NO_ERROR)
		return FALSE;

	if (LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) 
	{
		WSACleanup();
		return false;
	}

	return true;
}

bool MGServerEngine::Start(char* szIP, unsigned short Port)
{
	m_pServerReceiver = new MGServerReceiver(this);
	m_pServer = new AsynchIOService(m_pServerReceiver, 1024, 5, "MGServer");
	
	AsynchSocket* pProtoType = AsynchSocket::GetASPrototype();
	m_pAcceptor = new Acceptor(m_pServer, pProtoType, szIP, Port);
	m_pServer->start();

	m_pAcceptor->start();

	return true;
}

bool MGServerEngine::Shutdown()
{
	m_pAcceptor->stop();
	delete m_pAcceptor;

	m_pServer->stop();
	delete m_pServer;

	delete m_pServerReceiver;

	WSACleanup();

	return true;
}

bool MGServerEngine::SendRequest(BasePacket* pPacket)
{
	m_pServerReceiver->SendRequest(pPacket);
	return true;
}

bool MGServerEngine::Disconnect(int Serial)
{
	m_pServerReceiver->Disconnect(Serial);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
MGClientEngine::MGClientEngine(IEngine* pEngine)
	: INetworkEngine(pEngine)
{
}


MGClientEngine::~MGClientEngine(void)
{
}

bool MGClientEngine::Init()
{
	WSADATA wsaData;
	WORD wVer = MAKEWORD(2,2);    
	if (WSAStartup(wVer,&wsaData) != NO_ERROR)
		return FALSE;

	if (LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) 
	{
		WSACleanup();
		return false;
	}

	return true;
}

bool MGClientEngine::Start(char* szIP, unsigned short Port)
{
	m_pClientReceiver = new MGClientReceiver(this);
	m_pService = new AsynchIOService(m_pClientReceiver, 1024, 1, "MGServer");

	AsynchSocket* pProtoType = AsynchSocket::GetASPrototype();
	m_pService->connectSocket(1, pProtoType->clone(), szIP, Port);
	m_pService->start();

	return true;
}

bool MGClientEngine::Shutdown()
{
	m_pService->stop();
	delete m_pService;

	delete m_pClientReceiver;

	WSACleanup();

	return true;
}

bool MGClientEngine::SendRequest(BasePacket* pPacket)
{
	m_pClientReceiver->SendRequest(pPacket);
	return true;
}

bool MGClientEngine::Disconnect(int Serial)
{
	m_pClientReceiver->Disconnect(Serial);
	return true;
}