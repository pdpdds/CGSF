#include "stdafx.h"
#include "SFEngine.h"
#include "ILogicDispatcher.h"
#include "ace/os_ns_thread.h"
#include "SFBridgeThread.h"
#include "SFSessionService.h"

SFEngine::SFEngine(TCHAR* pArg)
	: m_LogicThreadId(-1)
	, m_PacketSendThreadId(-1)
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
		m_LogicThreadId = ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)BusinessThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1001);

		LogicEntrySingleton::instance()->SetLogic(pLogic);

		return TRUE;
	}

	return FALSE;
}

BOOL SFEngine::CreatePacketSendThread()
{	
	m_PacketSendThreadId = ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)PacketSendThread, this, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1002);

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


	LOG(INFO) << "Engine Initialize Complete!! ";
	return TRUE;

	/*int MaxPacketPool = 1000;

	PacketPoolSingleton::instance()->Init(MaxPacketPool);*/

}

////////////////////////////////////////////////////////////////////
//Add Timer
////////////////////////////////////////////////////////////////////
BOOL SFEngine::AddTimer(int timerID, DWORD period, DWORD delay)
{
	_TimerInfo Timer;
	Timer.TimerID = timerID;
	Timer.Period = period;
	Timer.StartDelay = delay;

	if(GetNetworkEngine()->CheckTimerImpl())
	{
		if(FALSE == GetNetworkEngine()->CreateTimerTask(timerID, delay, period))
		{
			LOG(ERROR) << "Timer Creation FAIL!!";
			return FALSE;
		}

		LOG(INFO) << "Timer Creation Success!!";
	}

	return TRUE;
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

BOOL extern gServerEnd;

BOOL SFEngine::ShutDown()
{
	gServerEnd = TRUE;

	SFPacket* pPacket = PacketPoolSingleton::instance()->Alloc();
	pPacket->SetPacketType(SFPACKET_SERVERSHUTDOWN);
	PacketSendSingleton::instance()->PushPacket(pPacket);

	BasePacket* pCommand = PacketPoolSingleton::instance()->Alloc();
	pCommand->SetOwnerSerial(-1);
	pCommand->SetPacketType(SFPACKET_SERVERSHUTDOWN);
	LogicGatewaySingleton::instance()->PushPacket(pCommand);

	ACE_Thread_Manager::instance()->wait_grp(m_LogicThreadId);
	ACE_Thread_Manager::instance()->wait_grp(m_PacketSendThreadId);

	m_pNetworkEngine->Shutdown();

	ACE::fini();

	google::ShutdownGoogleLogging();

	return TRUE;
}

bool SFEngine::OnConnect(int Serial)
{
	BasePacket* pPacket = new BasePacket();
	pPacket->SetPacketType(SFPACKET_CONNECT);
	pPacket->SetOwnerSerial(Serial);

	m_pLogicDispatcher->Dispatch(pPacket);
	
	return true;
}

bool SFEngine::OnDisconnect(int Serial)
{
	BasePacket* pPacket = new BasePacket();
	pPacket->SetPacketType(SFPACKET_DISCONNECT);
	pPacket->SetOwnerSerial(Serial);

	m_pLogicDispatcher->Dispatch(pPacket);
	
	return true;
}

bool SFEngine::OnTimer(const void *arg)
{
	BasePacket* pPacket = new BasePacket();
	pPacket->SetPacketType(SFPACKET_TIMER);
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
	pClonePacket->SetPacketType(SFPACKET_DATA);
	pClonePacket->SetOwnerSerial(pPacket->GetOwnerSerial());

	return PacketSendSingleton::instance()->PushPacket(pClonePacket);
}