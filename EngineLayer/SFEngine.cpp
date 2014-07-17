#include "stdafx.h"
#include "SFEngine.h"
#include "ILogicDispatcher.h"
#include "ace/os_ns_thread.h"
#include "SFBridgeThread.h"
#include "SFSessionService.h"
#include "SFCasualGameDispatcher.h"
#include "SFUtil.h"
#include "SFPacketDelaySendTask.h"
#include "SFServerConnectionManager.h"

#pragma comment(lib, "BaseLayer.lib")
#pragma comment(lib, "DatabaseLayer.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "liblzf.lib")
#pragma comment(lib, "libprotobuf.lib")

SFEngine* SFEngine::m_pEngine = NULL;

SFEngine::SFEngine()
	: m_packetSendThreadId(-1)
	, m_pNetworkEngine(0)
	, m_isServer(false)
{
	ACE::init();

	PacketDelayedSendTask::instance()->Init(100);

	google::InitGoogleLogging("CGSF");
	m_Config.Read(L"EngineConfig.xml");
	
	m_EngineHandle = 0;
}

SFEngine::~SFEngine(void)
{
	if(m_pNetworkEngine)
		delete m_pNetworkEngine;

	if (m_pServerConnectionManager)
		delete m_pServerConnectionManager;
}

SFEngine* SFEngine::GetInstance()
{
	if (m_pEngine == NULL)
		m_pEngine = new SFEngine();

	return m_pEngine;
}

bool SFEngine::CreateEngine(char* szModuleName, bool server)
{
	m_isServer = server;

	m_EngineHandle = ::LoadLibraryA(szModuleName);

	if(m_EngineHandle == 0)
		return false;

	CREATENETWORKENGINE *pfunc;
	pfunc = (CREATENETWORKENGINE*)::GetProcAddress( m_EngineHandle, "CreateNetworkEngine");
	m_pNetworkEngine = pfunc(server, this);

	if(m_pNetworkEngine == NULL)
		return false;

	if(FALSE == m_pNetworkEngine->Init())
		return false;
	
	return true;
}

void SFEngine::AddRPCService(IRPCService* pService)
{
	m_pLogicDispatcher->AddRPCService(pService);
	
}

bool SFEngine::CreatePacketSendThread()
{	
	m_packetSendThreadId = ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)PacketSendThread, this, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1002);

	return TRUE;
}

ISessionService* SFEngine::CreateSessionService()
{
	IPacketProtocol* pProtocol = m_pPacketProtocol->Clone();
	ISessionService* pService = new SFSessionService(pProtocol);
	return pService;
}

void SFEngine::SetLogFolder(TCHAR* szPath)
{
	WCHAR szFilePath[MAX_PATH] = { 0, };
	GetModuleFileName(NULL, szFilePath, MAX_PATH);

	WCHAR* path = SFUtil::ExtractPathInfo(szFilePath, SFUtil::PATH_DIR);
	SetCurrentDirectory(path);

	std::wstring szLogPath = path;
	szLogPath += L"Log\\";

	CreateDirectory(szLogPath.c_str(), NULL);

	if (szPath)
		szLogPath = szPath;

	google::SetLogDestination(google::GLOG_INFO, (char*)StringConversion::ToASCII(szLogPath).c_str());
	// INFO와 같은 폴더를 정의해서 파일 중복으로 경고나 에러 로그를 찍을 때 에러 발생.
	//google::SetLogDestination(google::GLOG_WARNING, (char*)StringConversion::ToASCII(szLogPath).c_str());
	//google::SetLogDestination(google::GLOG_ERROR, (char*)StringConversion::ToASCII(szLogPath).c_str());

	LOG(INFO) << "Log Destination " << (char*)StringConversion::ToASCII(szLogPath).c_str();
}

bool SFEngine::Intialize(ILogicEntry* pLogicEntry, IPacketProtocol* pProtocol, ILogicDispatcher* pDispatcher)
{
	LOG(INFO) << "Engine Initialize... ";

	SetLogFolder();

	ACE::init();

	ASSERT(pProtocol != NULL);

	SetPacketProtocol(pProtocol);

	if (pDispatcher == NULL)
	{
		pDispatcher = new SFCasualGameDispatcher();
	}

	SetLogicDispathcer(pDispatcher);

	m_pServerConnectionManager = new SFServerConnectionManager();

	LOG(INFO) << "Logic Entry Initialize";
	if (false == pLogicEntry->Initialize())
	{
		LOG(ERROR) << "LogicEntry Intialize Fail!!";
		return false;
	}
	
	_EngineConfig* pInfo = m_Config.GetConfigureInfo();
	
	std::string szNetworkEngineName = StringConversion::ToASCII(pInfo->EngineName);
	LOG(INFO) << "NetworkEngine Create : " << szNetworkEngineName.c_str();
	
	if (false == CreateEngine((char*)szNetworkEngineName.c_str(), true))
	{
		LOG(ERROR) << "NetworkEngine : " << szNetworkEngineName.c_str() << " Creation FAIL!!";
		return false;
	}

	LOG(INFO) << "NetworkEngine : " << szNetworkEngineName.c_str() << " Creation Success!!";

	if (false == pDispatcher->CreateLogicSystem(pLogicEntry))
	{
		LOG(ERROR) << "Logic System Creation FAIL!!";
		return false;
	}

	CreatePacketSendThread();

	LOG(INFO) << "Engine Initialize Complete!! ";
	return true;
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

bool SFEngine::Start(char* szIP, unsigned short port)
{
	_EngineConfig* pInfo = m_Config.GetConfigureInfo();

	LOG(INFO) << "Engine Starting... IP : " << (char*)StringConversion::ToASCII(pInfo->ServerIP).c_str() << " Port : " << pInfo->ServerPort;
	
	bool bResult = false;
	if (port != 0)
		bResult = m_pNetworkEngine->Start(szIP, port);
	else
		bResult = m_pNetworkEngine->Start((char*)StringConversion::ToASCII(pInfo->ServerIP).c_str(), pInfo->ServerPort);
	
	if (bResult == false)
	{
		LOG(ERROR) << "Engine Start Fail!!";
		return false;
	}
	LOG(INFO) << "Engine Start!!";

	return true;
}

bool SFEngine::ShutDown()
{
	LOG(INFO) << "Engine Shut Down!!";

	google::FlushLogFiles(google::GLOG_INFO);

	m_pLogicDispatcher->ShutDownLogicSystem();

	PacketSendSingleton::instance()->PushTask(NULL);
	
	ACE_Thread_Manager::instance()->wait_grp(m_packetSendThreadId);

	m_pNetworkEngine->Shutdown();

	ACE::fini();

	google::ShutdownGoogleLogging();

	delete this;

	return true;
}

bool SFEngine::OnConnect(int serial, int acceptorId)
{
	BasePacket* pPacket = new BasePacket();
	pPacket->SetAcceptorId(acceptorId);
	pPacket->SetPacketType(SFPACKET_CONNECT);
	pPacket->SetSerial(serial);

	m_pLogicDispatcher->Dispatch(pPacket);
	
	return true;
}

bool SFEngine::OnDisconnect(int serial, int acceptorId)
{
	BasePacket* pPacket = new BasePacket();
	pPacket->SetAcceptorId(acceptorId);
	pPacket->SetPacketType(SFPACKET_DISCONNECT);
	pPacket->SetSerial(serial);

	m_pLogicDispatcher->Dispatch(pPacket);
	
	return true;
}

bool SFEngine::OnTimer(const void *arg)
{
	UNREFERENCED_PARAMETER(arg);

	BasePacket* pPacket = new BasePacket();
	pPacket->SetPacketType(SFPACKET_TIMER);
	pPacket->SetSerial(-1);

	m_pLogicDispatcher->Dispatch(pPacket);

	return true;
}

bool SFEngine::SendRequest(BasePacket* pPacket)
{
	return GetPacketProtocol()->SendRequest(pPacket);
}

bool SFEngine::SendDelayedRequest(BasePacket* pPacket, std::vector<int>* pOwnerList)
{
	 SFPacketDelaySendTask* pTask = PacketDelayedSendTask::instance()->Alloc();
	 SFASSERT(NULL != pTask);

	if (pOwnerList == NULL)
	{
		std::vector<int> owner;
		owner.push_back(pPacket->GetSerial());
		pTask->SetPacket(pPacket, owner);
	}
	else
	{
		pTask->SetPacket(pPacket, *pOwnerList);
	}

	return PacketSendSingleton::instance()->PushTask(pTask);
}

bool SFEngine::SendRequest(BasePacket* pPacket, std::vector<int>& ownerList)
{
	auto iter = ownerList.begin();
	for (; iter != ownerList.end(); iter++)
	{
		pPacket->SetSerial(*iter);
		if (false == GetPacketProtocol()->SendRequest(pPacket))
		{
			DLOG(ERROR) << "broad cast fail";
		}
	}

	return true;
}

bool SFEngine::SendInternal(int ownerSerial, char* buffer, unsigned int bufferSize)
{
	return GetNetworkEngine()->SendInternal(ownerSerial, buffer, bufferSize);
}

bool SFEngine::ReleasePacket(BasePacket* pPacket)
{
	return m_pPacketProtocol->DisposePacket(pPacket);
}

int SFEngine::AddConnector(char* szIP, unsigned short port)
{
	return GetNetworkEngine()->AddConnector(szIP, port);
}

int SFEngine::AddListener(char* szIP, unsigned short port)
{
	return GetNetworkEngine()->AddListener(szIP, port);
}

bool SFEngine::LoadConnectionServerList(WCHAR* szFileName)
{
	return m_pServerConnectionManager->LoadConnectionServerList(szFileName);
}