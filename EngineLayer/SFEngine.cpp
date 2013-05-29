#include "stdafx.h"
#include "SFEngine.h"
#include "ILogicDispatcher.h"
#include "ace/os_ns_thread.h"
#include "SFBridgeThread.h"
#include "SFSessionService.h"
#include "P2PServer.h"

HINSTANCE g_pP2PHandle = 0;

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

BOOL SFEngine::Intialize(ILogicEntry* pLogicEntry, IPacketProtocol* pProtocol, ILogicDispatcher* pDispatcher)
{
	SetPacketProtocol(pProtocol);
	SetLogicDispathcer(pDispatcher);

	m_Config.Read(L"EngineConfig.xml");
	_EngineConfig* pInfo = m_Config.GetConfigureInfo();

	if(FALSE == CreateEngine((char*)StringConversion::ToASCII(pInfo->EngineName).c_str(), true))
		return 0;

	if(FALSE == CreateLogicThread(pLogicEntry))
		return 0;

	////////////////////////////////////////////////////////////////////
//Timer
////////////////////////////////////////////////////////////////////
	_TimerInfo Timer;
	Timer.TimerID = TIMER_1_SEC;
	Timer.Period = 1000;
	Timer.StartDelay = 5000;

	if(GetNetworkEngine()->CheckTimerImpl())
	{
		GetNetworkEngine()->CreateTimerTask(TIMER_1_SEC, 5000, 1000);
	}

	g_pP2PHandle = ::LoadLibrary(L"P2PServer.dll");

	if(g_pP2PHandle == NULL)
		return 0;

	ACTIVATEP2P_FUNC *pfuncActivate;
	pfuncActivate = (ACTIVATEP2P_FUNC *)::GetProcAddress( g_pP2PHandle, "ActivateP2P" );
	int Result = pfuncActivate();

	if(Result != 0)
		return 0;

	return TRUE;

	/*int MaxPacketPool = 1000;

	PacketPoolSingleton::instance()->Init(MaxPacketPool);*/

	/*SFConfigure Configure;
	_EngineConfig* pInfo = Configure.GetConfigureInfo();
	pInfo->EngineName = L"CGSFEngine.dll";
	pInfo->P2PModuleName = L"P2PServer.dll";
	pInfo->ServerIP = L"127.0.0.1";
	pInfo->ServerPort = 25251;
	pInfo->PacketProtocol = L"Protobuf";
	pInfo->HostName = L"Juhang";
	pInfo->TimerList.push_back(TIMER_1_SEC);
	Configure.Write(L"EngineConfig.xml");*/

}

BOOL SFEngine::Start()
{
	m_pNetworkEngine->Init();

	return m_pNetworkEngine->Start((char*)StringConversion::ToASCII(m_Config.GetConfigureInfo()->ServerIP)
		                         , m_Config.GetConfigureInfo()->ServerPort);
}

BOOL SFEngine::ShutDown()
{
	if(g_pP2PHandle)
	{
		DEACTIVATEP2P_FUNC *pfuncDeactivate;
		pfuncDeactivate = (DEACTIVATEP2P_FUNC *)::GetProcAddress( g_pP2PHandle, "DeactivateP2P" );
		int Result = pfuncDeactivate();

		::FreeLibrary(g_pP2PHandle);
	}

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