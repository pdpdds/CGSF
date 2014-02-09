#include "stdafx.h"
#include "SFEngine.h"
#include "ILogicDispatcher.h"
#include "ace/os_ns_thread.h"
#include "SFBridgeThread.h"
#include "SFSessionService.h"
#include "P2PServer.h"

HINSTANCE g_pP2PServerHandle = 0;

SFEngine::SFEngine(TCHAR* pArg)
{
	/*SFConfigure Configure;
	_EngineConfig* pInfo = Configure.GetConfigureInfo();
	pInfo->EngineName = L"CGSFEngine.dll";
	pInfo->P2PModuleName = L"P2PServer.dll";
	pInfo->ServerIP = L"127.0.0.1";
	pInfo->ServerPort = 25251;
	pInfo->PacketProtocol = L"Protobuf";
	pInfo->HostName = L"Juhang";
	pInfo->TimerList.push_back(TIMER_1_SEC);
	pInfo->LogDirectory = L"d:\\cgsflog\\";
	Configure.Write(L"EngineConfig.xml");

	Configure.Write(L"EngineConfig.xml");*/
	google::InitGoogleLogging((char*)StringConversion::ToASCII(pArg).c_str());
	
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

	if(FALSE == m_pNetworkEngine->Init())
		return FALSE;

	CreatePacketSendThread();
	
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

BOOL SFEngine::CreatePacketSendThread()
{	
	ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)PacketSendThread, this, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2);

	return TRUE;
}

ISessionService* SFEngine::CreateSessionService()
{
	IPacketProtocol* pProtocol = m_pPacketProtocol->Clone();
	ISessionService* pService = new SFSessionService(pProtocol);
	return pService;
}

BOOL SFEngine::Intialize(ILogicEntry* pLogicEntry, IPacketProtocol* pProtocol, ILogicDispatcher* pDispatcher)
{
	LOG(INFO) << "Engine Initialize... ";
	
	m_Config.Read(L"EngineConfig.xml");
	_EngineConfig* pInfo = m_Config.GetConfigureInfo();

	google::SetLogDestination(google::GLOG_INFO, (char*)StringConversion::ToASCII(pInfo->LogDirectory).c_str()); 
	google::SetLogDestination(google::GLOG_WARNING, (char*)StringConversion::ToASCII(pInfo->LogDirectory).c_str()); 
	google::SetLogDestination(google::GLOG_ERROR, (char*)StringConversion::ToASCII(pInfo->LogDirectory).c_str()); 

	LOG(INFO) << "Log Destination " << (char*)StringConversion::ToASCII(pInfo->LogDirectory).c_str();

	ACE::init();
	LOG(INFO) << "ACE Init ";

	ASSERT(pProtocol != NULL);
	ASSERT(pDispatcher != NULL);

	SetPacketProtocol(pProtocol);
	SetLogicDispathcer(pDispatcher);

	LOG(INFO) << "PacketProtocol Setting";
	LOG(INFO) << "LogicDispatcher Setting";

	if(FALSE == pLogicEntry->Initialize())
	{
		LOG(ERROR) << "LogicEntry Intialize Fail!!";
		return FALSE;
	}

	LOG(INFO) << "LogicEntry Intialize Success!!";

	if(FALSE == CreateEngine((char*)StringConversion::ToASCII(pInfo->EngineName).c_str(), true))
	{
		LOG(ERROR) << "NetworkEngine : " << StringConversion::ToASCII(pInfo->EngineName).c_str() << " Creation FAIL!!";
		return FALSE;
	}

	LOG(INFO) << "NetworkEngine : " << StringConversion::ToASCII(pInfo->EngineName).c_str() << " Creation Success!!";

	if(FALSE == CreateLogicThread(pLogicEntry))
	{
		LOG(ERROR) << "LogicThread Creation FAIL!!";
		return FALSE;
	}

	LOG(INFO) << "LogicThread Creation Success!!";

	////////////////////////////////////////////////////////////////////
//Timer
////////////////////////////////////////////////////////////////////
	_TimerInfo Timer;
	Timer.TimerID = TIMER_1_SEC;
	Timer.Period = 1000;
	Timer.StartDelay = 5000;

	if(GetNetworkEngine()->CheckTimerImpl())
	{
		if(FALSE == GetNetworkEngine()->CreateTimerTask(TIMER_1_SEC, 5000, 1000))
		{
			LOG(ERROR) << "Timer Creation FAIL!!";
			return FALSE;
		}

		LOG(INFO) << "Timer Creation Success!!";
	}


	if(pInfo->P2PModuleName.empty() != TRUE)
	{
		g_pP2PServerHandle = ::LoadLibrary(L"P2PServer.dll");

		std::string p2pName = StringConversion::ToASCII(pInfo->P2PModuleName.c_str());

		if(g_pP2PServerHandle == NULL)
		{
			LOG(ERROR) << "P2P Module" << p2pName << " Handle Create Fail!!";
			return FALSE;
		}

		ACTIVATEP2P_FUNC *pfuncActivate;
		pfuncActivate = (ACTIVATEP2P_FUNC *)::GetProcAddress( g_pP2PServerHandle, "ActivateP2P" );

		if(pfuncActivate == NULL)
		{
			LOG(ERROR) << "P2P Module " << p2pName << " => Can't find ActivateP2P Method!!";
			return FALSE;
		}

		int Result = pfuncActivate();

		if(Result != 0)
		{
			LOG(ERROR) << "P2P Module " << p2pName << " Activate fail!!";
			return FALSE;
		}

		LOG(INFO) << "P2P Module " << p2pName << " Initialize Complete";
	}

	LOG(INFO) << "Engine Initialize Complete!! ";
	return TRUE;

	/*int MaxPacketPool = 1000;

	PacketPoolSingleton::instance()->Init(MaxPacketPool);*/

}

BOOL SFEngine::Start()
{
	_EngineConfig* pInfo = m_Config.GetConfigureInfo();

	LOG(INFO) << "Engine Starting... IP : " << (char*)StringConversion::ToASCII(pInfo->ServerIP).c_str() << " Port : " << pInfo->ServerPort;
	
	if(false == m_pNetworkEngine->Start((char*)StringConversion::ToASCII(pInfo->ServerIP).c_str(), pInfo->ServerPort))
	{
		LOG(ERROR) << "Engine Start Fail!!";
		return false;
	}

	LOG(INFO) << "Engine Start!!";

	return true;
}

BOOL SFEngine::Start(char* szIP, unsigned short Port)
{
	//m_pNetworkEngine->Init();

	return m_pNetworkEngine->Start(szIP, Port);
}

BOOL SFEngine::ShutDown()
{
	if(g_pP2PServerHandle)
	{
		DEACTIVATEP2P_FUNC *pfuncDeactivate;
		pfuncDeactivate = (DEACTIVATEP2P_FUNC *)::GetProcAddress( g_pP2PServerHandle, "DeactivateP2P" );
		int Result = pfuncDeactivate();

		::FreeLibrary(g_pP2PServerHandle);
	}

	m_pNetworkEngine->Shutdown();

	ACE::fini();

	google::ShutdownGoogleLogging();

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

	return true;
}

BOOL SFEngine::SendRequest(BasePacket* pPacket)
{
	//return GetNetworkEngine()->SendRequest(pPacket);

	SFPacket* pClonePacket = PacketPoolSingleton::instance()->Alloc();

	unsigned int writtenSize;
	bool result = m_pPacketProtocol->GetPacketData(pPacket, (char*)pClonePacket->GetDataBuffer(), MAX_PACKET_DATA, writtenSize);

	if(writtenSize == 0)
	{
		PacketPoolSingleton::instance()->Release(pClonePacket);
		return FALSE;
	}

	pClonePacket->SetDataSize(writtenSize);
	pClonePacket->SetPacketType(SFPacket_Data);
	pClonePacket->SetOwnerSerial(pPacket->GetOwnerSerial());

	return PacketSendSingleton::instance()->PushPacket(pClonePacket);
}