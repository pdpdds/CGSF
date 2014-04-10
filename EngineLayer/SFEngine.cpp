#include "stdafx.h"
#include "SFEngine.h"
#include "ILogicDispatcher.h"
#include "ace/os_ns_thread.h"
#include "SFBridgeThread.h"
#include "SFSessionService.h"
#include "SFCasualGameDispatcher.h"
#include "SFUtil.h"

#pragma comment(lib, "BaseLayer.lib")
#pragma comment(lib, "DatabaseLayer.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "liblzf.lib")
#pragma comment(lib, "libprotobuf.lib")

SFEngine* SFEngine::m_pEngine = NULL;

SFEngine::SFEngine()
	: m_LogicThreadId(-1)
	, m_PacketSendThreadId(-1)
	, m_bServerTerminated(false)
	, m_pNetworkEngine(0)
{
	ACE::init();
	google::InitGoogleLogging("CGSF");
	m_Config.Read(L"EngineConfig.xml");
	
	m_EngineHandle = 0;
}

SFEngine::~SFEngine(void)
{
	if(m_pNetworkEngine)
		delete m_pNetworkEngine;
}

SFEngine* SFEngine::GetInstance()
{
	if (m_pEngine == NULL)
		m_pEngine = new SFEngine();

	return m_pEngine;
}

bool SFEngine::CreateEngine(char* szModuleName, bool Server)
{
	m_EngineHandle = ::LoadLibraryA(szModuleName);

	if(m_EngineHandle == 0)
		return false;

	CREATENETWORKENGINE *pfunc;
	pfunc = (CREATENETWORKENGINE*)::GetProcAddress( m_EngineHandle, "CreateNetworkEngine");
	m_pNetworkEngine = pfunc(Server, this);

	if(m_pNetworkEngine == NULL)
		return false;

	if(FALSE == m_pNetworkEngine->Init())
		return false;

	CreatePacketSendThread();
	
	return true;
}

bool SFEngine::CreateLogicThread(ILogicEntry* pLogic)
{
	if(pLogic != NULL)
	{
		m_LogicThreadId = ACE_Thread_Manager::instance()->spawn_n(m_pLogicDispatcher->GetLogicThreadCount(), (ACE_THR_FUNC)m_pLogicDispatcher->GetBusinessThreadFunc(), this, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1001);

		LogicEntrySingleton::instance()->SetLogic(pLogic);

		return true;
	}

	return false;
}

bool SFEngine::CreatePacketSendThread()
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

 void SFEngine::SetLogFolder()
{
	WCHAR szFilePath[MAX_PATH] = { 0, };
	GetModuleFileName(NULL, szFilePath, MAX_PATH);

	WCHAR* szPath = SFUtil::ExtractPathInfo(szFilePath, SFUtil::PATH_DIR);
	SetCurrentDirectory(szPath);

	std::wstring szLogPath = szPath;
	szLogPath += L"Log\\";

	google::SetLogDestination(google::GLOG_INFO, (char*)StringConversion::ToASCII(szLogPath).c_str());
	google::SetLogDestination(google::GLOG_WARNING, (char*)StringConversion::ToASCII(szLogPath).c_str());
	google::SetLogDestination(google::GLOG_ERROR, (char*)StringConversion::ToASCII(szLogPath).c_str());

	LOG(INFO) << "Log Destination " << (char*)StringConversion::ToASCII(szLogPath).c_str();
}

bool SFEngine::Intialize(ILogicEntry* pLogicEntry, IPacketProtocol* pProtocol, ILogicDispatcher* pDispatcher)
{
	SetLogFolder();

	LOG(INFO) << "Engine Initialize... ";

	ACE::init();
	LOG(INFO) << "ACE Init ";

	ASSERT(pProtocol != NULL);

	SetPacketProtocol(pProtocol);

	if (pDispatcher == NULL)
	{
		pDispatcher = new SFCasualGameDispatcher();
	}

	SetLogicDispathcer(pDispatcher);

	LOG(INFO) << "PacketProtocol Setting";
	LOG(INFO) << "LogicDispatcher Setting";

	if (false == pLogicEntry->Initialize())
	{
		LOG(ERROR) << "LogicEntry Intialize Fail!!";
		return false;
	}

	LOG(INFO) << "LogicEntry Intialize Success!!";

	_EngineConfig* pInfo = m_Config.GetConfigureInfo();
	if (false == CreateEngine((char*)StringConversion::ToASCII(pInfo->EngineName).c_str(), true))
	{
		LOG(ERROR) << "NetworkEngine : " << StringConversion::ToASCII(pInfo->EngineName).c_str() << " Creation FAIL!!";
		return false;
	}

	LOG(INFO) << "NetworkEngine : " << StringConversion::ToASCII(pInfo->EngineName).c_str() << " Creation Success!!";

	if(FALSE == CreateLogicThread(pLogicEntry))
	{
		LOG(ERROR) << "LogicThread Creation FAIL!!";
		return false;
	}

	LOG(INFO) << "LogicThread Creation Success!!";


	LOG(INFO) << "Engine Initialize Complete!! ";
	return true;

	/*int MaxPacketPool = 1000;

	PacketPoolSingleton::instance()->Init(MaxPacketPool);*/

}

////////////////////////////////////////////////////////////////////
//Add Timer
////////////////////////////////////////////////////////////////////
bool SFEngine::AddTimer(int timerID, DWORD period, DWORD delay)
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

bool SFEngine::Start()
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

bool SFEngine::Start(char* szIP, unsigned short Port)
{
	//m_pNetworkEngine->Init();

	return m_pNetworkEngine->Start(szIP, Port);
}

bool SFEngine::ShutDown()
{
	LOG(INFO) << "Engine Shut Down!!";

	google::FlushLogFiles(google::GLOG_INFO);

	m_bServerTerminated = true;

	BasePacket* pCommand = PacketPoolSingleton::instance()->Alloc();
	pCommand->SetPacketType(SFPACKET_SERVERSHUTDOWN);
	PacketSendSingleton::instance()->PushPacket(pCommand);

	pCommand = PacketPoolSingleton::instance()->Alloc();
	pCommand->SetOwnerSerial(-1);
	pCommand->SetPacketType(SFPACKET_SERVERSHUTDOWN);
	LogicGatewaySingleton::instance()->PushPacket(pCommand);

	ACE_Thread_Manager::instance()->wait_grp(m_LogicThreadId);
	ACE_Thread_Manager::instance()->wait_grp(m_PacketSendThreadId);

	m_pNetworkEngine->Shutdown();

	ACE::fini();

	google::ShutdownGoogleLogging();

	delete this;

	return true;
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

bool SFEngine::SendDelayedRequest(BasePacket* pPacket)
{
	SFPacket* pClonePacket = PacketPoolSingleton::instance()->Alloc();

	unsigned int writtenSize;
	bool result = m_pPacketProtocol->GetPacketData(pPacket, (char*)pClonePacket->GetHeader(), MAX_PACKET_DATA, writtenSize);

	if(writtenSize == 0)
	{
		PacketPoolSingleton::instance()->Release(pClonePacket);
		return false;
	}

	pClonePacket->SetDataSize(writtenSize);
	pClonePacket->SetPacketType(SFPACKET_DATA);
	pClonePacket->SetOwnerSerial(pPacket->GetOwnerSerial());

	return PacketSendSingleton::instance()->PushPacket(pClonePacket);
}

bool SFEngine::SendRequest(BasePacket* pPacket)
{
	return GetPacketProtocol()->SendRequest(pPacket);
}

bool SFEngine::SendInternal(int ownerSerial, char* buffer, unsigned int bufferSize)
{
	return GetNetworkEngine()->SendInternal(ownerSerial, buffer, bufferSize);
}

bool SFEngine::ReleasePacket(BasePacket* pPacket)
{
	return m_pPacketProtocol->DisposePacket(pPacket);
}
