#include "stdafx.h"
#include "SFEngine.h"
#include "ILogicDispatcher.h"
#include "ace/os_ns_thread.h"
#include "SFBridgeThread.h"
#include "SFSessionService.h"

SFEngine::SFEngine(void)
{
	ACE::init();
	
	m_EngineHandle = 0;
}

SFEngine::~SFEngine(void)
{
	if(m_pNetworkEngine)
		delete m_pNetworkEngine;
}

BOOL SFEngine::CreateEngine(char* szModuleName, bool Server)
{
	m_EngineHandle = ::LoadLibraryA(szModuleName);

	if(m_EngineHandle == 0)
		return FALSE;

	CREATENETWORKENGINE *pfunc;
	pfunc = (CREATENETWORKENGINE*)::GetProcAddress( m_EngineHandle, "CreateNetworkEngine");
	m_pNetworkEngine = pfunc(Server, this);

	if(m_pNetworkEngine == NULL)
		return FALSE;
	
	return TRUE;
}

BOOL SFEngine::CreateLogicThread(ILogicEntry* pLogic)
{
	if(pLogic != NULL)
	{
		ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)BusinessThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2);

		LogicEntrySingleton::instance()->SetLogic(pLogic);

		return TRUE;
	}

	return FALSE;
}

ISessionService* SFEngine::CreateSessionService()
{
	IPacketProtocol* pProtocol = m_pPacketProtocol->Clone();
	ISessionService* pService = new SFSessionService(pProtocol);
	return pService;
}

BOOL SFEngine::Start(char* szIP, unsigned short Port)
{
	m_pNetworkEngine->Init();

	return m_pNetworkEngine->Start(szIP, Port);
}

BOOL SFEngine::ShutDown()
{
	m_pNetworkEngine->Shutdown();

	ACE::fini();

	return TRUE;
}

bool SFEngine::OnConnect(int Serial)
{
	BasePacket* pPacket = new BasePacket();
	pPacket->SetPacketType(SFPacket_Connect);
	pPacket->SetOwnerSerial(Serial);

	m_pLogicDispatcher->Dispatch(pPacket);
	
	return true;
}

bool SFEngine::OnDisconnect(int Serial)
{
	BasePacket* pPacket = new BasePacket();
	pPacket->SetPacketType(SFPacket_Disconnect);
	pPacket->SetOwnerSerial(Serial);

	m_pLogicDispatcher->Dispatch(pPacket);
	
	return true;
}

bool SFEngine::OnTimer(const void *arg)
{
	BasePacket* pPacket = new BasePacket();
	pPacket->SetPacketType(SFPacket_Timer);
	pPacket->SetOwnerSerial(-1);

	m_pLogicDispatcher->Dispatch(pPacket);

	printf("%d\n", arg);
	// Print out when timeouts occur.
	//  ACE_DEBUG ((LM_DEBUG, "(%t) %d timeout occurred for %s @ %d.\n",
	//        1,
	//        (char *) arg,
	//      (tv - this->start_time_).sec ()));

	return true;
}

BOOL SFEngine::SendRequest(BasePacket* pPacket)
{
	return GetNetworkEngine()->SendRequest(pPacket);
}