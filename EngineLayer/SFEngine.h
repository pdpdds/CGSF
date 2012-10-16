#pragma once
#include "SFFactory.h"
#include "IEngine.h"
#include "SFConfig.h"
#include "INetworkEngine.h"
#include "SFNetworkEngineCallback.h"
#include "ace/os_ns_thread.h"
#include "SFBridgeThread.h"

//class INetworkFramework;

using namespace CGSF;

template <typename LoggerPolicy, typename NetworkPolicy>
class SFEngine : public IEngine
{
public:
	SFEngine(void);
	virtual ~SFEngine(void);

	virtual BOOL CreateSystem(char* szModuleName, ILogicEntry* pLogic, bool Server = false) override;
	virtual BOOL Start(char* szIP, unsigned short Port) override;
	virtual BOOL ShutDown() override;

	NetworkPolicy* GetNetworkEngine(){return m_pNetworkEngine;}

	BOOL GetProcessing(){return m_Processing;}
	void SetProcessing(BOOL Processing){m_Processing = Processing;}

	static SFConfig* GetConfig(){return &m_Config;}

	BOOL Send( int Serial, int PacketID, char* pBuffer, int BufferSize )
	{
		int HeaderSize = sizeof(SFPacketHeader);

		SFPacket PacketSend;

		PacketSend.SetPacketID(PacketID);

		PacketSend.MakePacket((BYTE*)pBuffer, BufferSize, CGSF_PACKET_OPTION);

		return GetNetworkEngine()->Send(Serial, (char*)PacketSend.GetHeader(), PacketSend.GetHeaderSize() + PacketSend.GetDataSize());
	}

protected:
	NetworkPolicy* m_pNetworkEngine;
	LoggerPolicy* m_pLoggerPolicy;
	
private:
	//SFFactory<INetworkFramework> m_SvrFactory;
	BOOL m_Processing;

	static SFConfig m_Config;
	HINSTANCE m_EngineHandle;
};

template <typename LoggerPolicy, typename NetworkPolicy>
SFEngine<typename LoggerPolicy, typename NetworkPolicy>::SFEngine(void)
{
	ACE::init();
	//m_SvrFactory.Register(NetworkModulePolicy::GetModuleName(), (SFCreatorBase<INetworkFramework>*)(new SFCreator<INetworkFramework, NetworkModulePolicy>));
	m_EngineHandle = 0;
	m_pLoggerPolicy = new LoggerPolicy();

	m_Processing = FALSE;
}

template <typename LoggerPolicy, typename NetworkPolicy>
SFEngine<typename LoggerPolicy, typename NetworkPolicy>::~SFEngine(void)
{
	delete m_pLoggerPolicy;
	delete m_pNetworkEngine;
}

template <typename LoggerPolicy, typename NetworkPolicy>
BOOL SFEngine<typename LoggerPolicy, typename NetworkPolicy>::CreateSystem(char* szModuleName, ILogicEntry* pLogic, bool Server)
{
	//m_pNetworkFramework = m_SvrFactory.Create(NetworkModulePolicy::GetModuleName());
	if(pLogic != NULL)
	{
		ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)BusinessThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2);

		LogicEntrySingleton::instance()->SetLogic(pLogic);
	}

	m_EngineHandle = ::LoadLibraryA(szModuleName);

	if(m_EngineHandle == 0)
		return FALSE;

	
	INetworkEngineCallback* pCallback = new SFNetworkEngineCallback();

	CREATENETWORKENGINE *pfunc;
	pfunc = (CREATENETWORKENGINE*)::GetProcAddress( m_EngineHandle, "CreateNetworkEngine");
	m_pNetworkEngine = pfunc(Server, pCallback);

	if(m_pNetworkEngine == NULL)
		return FALSE;

	m_pNetworkEngine->Init();
	
	return TRUE;
}

template <typename LoggerPolicy, typename NetworkPolicy>
BOOL SFEngine<typename LoggerPolicy, typename NetworkPolicy>::Start(char* szIP, unsigned short Port)
{
	m_pLoggerPolicy->Initialize();

	m_Processing = TRUE;

	return m_pNetworkEngine->Start(szIP, Port);
}

template <typename LoggerPolicy, typename NetworkPolicy>
BOOL SFEngine<typename LoggerPolicy, typename NetworkPolicy>::ShutDown()
{
	m_pNetworkEngine->Shutdown();
	m_pLoggerPolicy->Finally();

	ACE::fini();

	return TRUE;
}