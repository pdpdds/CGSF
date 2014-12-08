#include "stdafx.h"
#include "SFEngine.h"
#include <stdlib.h>  
#include "ILogicDispatcher.h"
#include "ace/os_ns_thread.h"
#include "SFBridgeThread.h"
#include "SFSessionService.h"
#include "SFCasualGameDispatcher.h"
#include "SFUtil.h"
#include "SFPacketDelaySendTask.h"
#include "SFServerConnectionManager.h"
#include "SFPacketProtocolManager.h"

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
	, m_pPacketProtocolManager(NULL)
	, m_pServerConnectionManager(NULL)
{
	ACE::init();

	PacketDelayedSendTask::instance()->Init(100);

	google::InitGoogleLogging("CGSF");
	m_Config.Read(L"EngineConfig.xml");

#ifdef _DEBUG
	_putenv_s("GLOG_logbufsecs", "0");
	google::LogToStderr();
#endif
	
	m_engineHandle = 0;

	m_pPacketProtocolManager = new SFPacketProtocolManager();
}

SFEngine::~SFEngine(void)
{	
	ACE::fini();

	if (m_pNetworkEngine)
		delete m_pNetworkEngine;

	if (m_engineHandle)
		FreeLibrary(m_engineHandle);
}

SFEngine* SFEngine::GetInstance()
{
	if (m_pEngine == NULL)
		m_pEngine = new SFEngine();

	return m_pEngine;
}

NET_ERROR_CODE SFEngine::CreateEngine(char* szModuleName, bool server)
{
	m_isServer = server;

	m_engineHandle = ::LoadLibraryA(szModuleName);

	if (m_engineHandle == 0)
		return NET_ERROR_CODE::ENGINE_INIT_CREAT_ENGINE_LOAD_DLL_FAIL;

	CREATENETWORKENGINE *pfunc;
	pfunc = (CREATENETWORKENGINE*)::GetProcAddress(m_engineHandle, "CreateNetworkEngine");
	m_pNetworkEngine = pfunc(server, this);

	if(m_pNetworkEngine == NULL)
		return NET_ERROR_CODE::ENGINE_INIT_CREAT_ENGINE_FUNC_NULL;

	if(FALSE == m_pNetworkEngine->Init())
		return NET_ERROR_CODE::ENGINE_INIT_CREAT_ENGINE_INIT_FAIL;
	
	return NET_ERROR_CODE::SUCCESS;
}

void SFEngine::AddRPCService(IRPCService* pService)
{
	m_pLogicDispatcher->AddRPCService(pService);
	
}

bool SFEngine::CreatePacketSendThread()
{	
	m_packetSendThreadId = ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)PacketSendThread, this, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY);

	return TRUE;
}

ISessionService* SFEngine::CreateSessionService(_SessionDesc& desc)
{
	IPacketProtocol* pSourceProtocol = NULL;
	if (desc.sessionType == 0)
		pSourceProtocol = m_pPacketProtocolManager->GetPacketProtocolWithListenerId(desc.identifier);
	else
		pSourceProtocol = m_pPacketProtocolManager->GetPacketProtocolWithConnectorId(desc.identifier);

	IPacketProtocol* pCloneProtocol = pSourceProtocol->Clone();
	return new SFSessionService(pCloneProtocol);
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

NET_ERROR_CODE SFEngine::Intialize(ILogicEntry* pLogicEntry, ILogicDispatcher* pDispatcher)
{
	LOG(INFO) << "Engine Initialize... ";

	SetLogFolder();	

	if (NULL == pLogicEntry)
		return NET_ERROR_CODE::ENGINE_INIT_LOGIC_ENTRY_NULL;	

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
		return NET_ERROR_CODE::ENGINE_INIT_LOGIC_ENTRY_FAIL;
	}
	
	
	_EngineConfig* pInfo = m_Config.GetConfigureInfo();
	
	LOG(INFO) << "Basic Port: " << pInfo->serverPort;

	std::string szNetworkEngineName = StringConversion::ToASCII(pInfo->engineName);
	LOG(INFO) << "NetworkEngine Create : " << szNetworkEngineName.c_str();

	LOG(INFO) << "MaxAccept : " << pInfo->maxAccept;

	if (pInfo->maxAccept <= 0 || pInfo->maxAccept > MAX_USER_ACCEPT)
	{
		LOG(WARNING) << "MaxAccept Value Abnormal : " << pInfo->maxAccept;
		SetMaxUserAccept(DEFALUT_USER_ACCEPT);
	}
	else
		SetMaxUserAccept(pInfo->maxAccept);
		
	auto errorCode = CreateEngine((char*)szNetworkEngineName.c_str(), true);
	if (errorCode != NET_ERROR_CODE::SUCCESS)
	{
		LOG(ERROR) << "NetworkEngine : " << szNetworkEngineName.c_str() << " Creation FAIL!!";
		return errorCode;
	}

	LOG(INFO) << "NetworkEngine : " << szNetworkEngineName.c_str() << " Creation Success!!";

	if (false == pDispatcher->CreateLogicSystem(pLogicEntry))
	{
		LOG(ERROR) << "Logic System Creation FAIL!!";
		return NET_ERROR_CODE::ENGINE_INIT_CREAT_LOGIC_SYSTEM_FAIL;
	}

	CreatePacketSendThread();

	LOG(INFO) << "Engine Initialize Complete!! ";
	return NET_ERROR_CODE::SUCCESS;
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

	LOG(INFO) << "Engine Starting... IP : " << (char*)StringConversion::ToASCII(pInfo->serverIP).c_str() << " Port : " << pInfo->serverPort;	
	
	bool bResult = false;
	if (port != 0)
		bResult = m_pNetworkEngine->Start(szIP, port);
	else
		bResult = m_pNetworkEngine->Start((char*)StringConversion::ToASCII(pInfo->serverIP).c_str(), pInfo->serverPort);
	
	if (bResult == false)
	{
		LOG(ERROR) << "Engine Start Fail!!";
		return false;
	}
	LOG(INFO) << "Engine Start!!";

	return true;
}

bool SFEngine::Start(int protocolId)
{
	bool bResult = false;
	_EngineConfig* pInfo = m_Config.GetConfigureInfo();
	LOG(INFO) << "Engine Starting... IP : " << (char*)StringConversion::ToASCII(pInfo->serverIP).c_str() << " Port : " << pInfo->serverPort;

	if (protocolId >= 0)
	{
		IPacketProtocol* pProtocol = m_pPacketProtocolManager->GetPacketProtocol(protocolId);

		if (pProtocol == NULL)
		{
			LOG(ERROR) << "Engine Start Fail. PacketProtocol None";
			return false;
		}

		int listenerId = -1;
		
		if (pInfo->serverPort != 0)
		{
			listenerId = AddListener((char*)StringConversion::ToASCII(pInfo->serverIP).c_str(), pInfo->serverPort, protocolId);

			if (listenerId <= 0)
			{
				LOG(ERROR) << "Engine Start Fail. m_pNetworkEngine->AddListener fail";
				return false;
			}
		}
	}

	bResult = m_pNetworkEngine->Start((char*)StringConversion::ToASCII(pInfo->serverIP).c_str(), pInfo->serverPort);

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
	// 올바르게 종료되는지 조사하기 위해 각 단계별로 로그를 남깁니다.
	LOG(INFO) << "Engine Shut Down!!";	
		
	m_pLogicDispatcher->ShutDownLogicSystem();
	LOG(INFO) << "Engine Shut Down Step (1) ShutDownLogicSystem";	

	if (m_packetSendThreadId >= 0)
	{
		PacketSendSingleton::instance()->PushTask(NULL);
		LOG(INFO) << "Engine Shut Down Step (2) instance()->PushTask(NULL)";

		ACE_Thread_Manager::instance()->wait_grp(m_packetSendThreadId);
		LOG(INFO) << "Engine Shut Down Step (3) wait_grp(m_packetSendThreadId)";
	}

	if (m_pNetworkEngine)
	{
		m_pNetworkEngine->Shutdown();
		LOG(INFO) << "Engine Shut Down Step (4) m_pNetworkEngine->Shutdown()";		
				
		//delete m_pNetworkEngine;
	}
	
	if (m_pServerConnectionManager)
	{
		delete m_pServerConnectionManager;
		LOG(INFO) << "Engine Shut Down Step (5) delete Server Connecton Manager";
	}
	
	delete this;
	LOG(INFO) << "Engine Shut Down Step (6) Engine Delete";

	google::ShutdownGoogleLogging();
	
	return true;
}

bool SFEngine::OnConnect(int serial, _SessionDesc& desc)
{
	BasePacket* pPacket = new BasePacket();
	pPacket->SetSessionDesc(desc);
	pPacket->SetPacketType(SFPACKET_CONNECT);
	pPacket->SetSerial(serial);

	m_pLogicDispatcher->Dispatch(pPacket);
	
	return true;
}

bool SFEngine::OnDisconnect(int serial, _SessionDesc& desc)
{
	BasePacket* pPacket = new BasePacket();
	pPacket->SetSessionDesc(desc);
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
	return GetNetworkEngine()->SendRequest(pPacket);
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
		if (false == GetNetworkEngine()->SendRequest(pPacket))
		{
			DLOG(ERROR) << "broad cast fail";
		}
	}

	return true;
}

bool SFEngine::ReleasePacket(BasePacket* pPacket)
{
	pPacket->Release();
	return true;
}

bool SFEngine::Disconnect(int serial)
{
	return GetNetworkEngine()->Disconnect(serial);
}

int SFEngine::AddConnector(int connectorId, char* szIP, unsigned short port)
{
	return GetNetworkEngine()->AddConnector(connectorId, szIP, port);
}

int SFEngine::AddListener(char* szIP, unsigned short port, int packetProtocolId)
{
	int listenerId = GetNetworkEngine()->AddListener(szIP, port);

	if (listenerId)
	{
		m_pPacketProtocolManager->AddListenerInfo(listenerId, packetProtocolId);		
	}

	return listenerId;
}

bool SFEngine::LoadConnectorList(WCHAR* szFileName)
{
	return m_pServerConnectionManager->LoadConnectorList(szFileName);
}

bool SFEngine::SetupServerReconnectSys()
{
	return m_pServerConnectionManager->SetupServerReconnectSys();
}

bool SFEngine::AddPacketProtocol(int packetProtocolId, IPacketProtocol* pProtocol)
{
	return m_pPacketProtocolManager->AddPacketProtocol(packetProtocolId, pProtocol);
}