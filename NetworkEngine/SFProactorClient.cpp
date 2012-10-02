#include "StdAfx.h"
#include "SFProactorClient.h"
#include <ace/Singleton.h>
#include <ace/SOCK_Connector.h>
#include "SFProactorService.h"
#include "SFIni.h"
#include "SFBridgeThread.h"

extern BOOL gServerEnd;

SFProactorClient::SFProactorClient(void)
{
	SFIni ini;
	
	ini.SetPathName(_T("./Connection.ini"));
	ini.GetString(L"ServerInfo",L"IP",m_IP, 20);
	m_Port = ini.GetInt(L"ServerInfo",L"PORT",0);
}

SFProactorClient::~SFProactorClient(void)
{
}

BOOL SFProactorClient::Run()
{
	ACE_SOCK_Stream* stream = new ACE_SOCK_Stream();
	ACE_INET_Addr connectAddr(m_Port, m_IP);
	ACE_SOCK_Connector connector;
	int result = connector.connect(*stream, connectAddr);
	if (-1 == result)
		return false;

	m_pNetworkService = new SFProactorService();
	m_pNetworkService->SetOwner(this);

	ACE_Message_Block mb;
	m_pNetworkService->open(stream->get_handle(), mb);
	delete stream;
	stream = NULL;

	return TRUE;
}

BOOL SFProactorClient::Start( ILogicEntry* pLogic )
{
	int OptimalThreadCount = 1;

	m_WorkThreadGroupID = ACE_Thread_Manager::instance()->spawn_n(OptimalThreadCount, (ACE_THR_FUNC)ProactiveWorkThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1);

	if(m_WorkThreadGroupID == -1)
	{
		SFASSERT(0);
		return FALSE;
	}

	if(pLogic)
	{
		ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)BusinessThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2);
		LogicEntrySingleton::instance()->SetLogic(pLogic);
	}

	return Run();
}

BOOL SFProactorClient::End()
{
	gServerEnd = TRUE;

	ACE_Thread_Manager::instance()->wait_grp(m_WorkThreadGroupID);

	return TRUE;
}

BOOL SFProactorClient::ServiceDisconnect( int Serial )
{
	ProactorServiceMapSingleton::instance()->UnRegister(Serial);

	SFPacket* pCommand = PacketPoolSingleton::instance()->Alloc();
	pCommand->SetPacketType(SFCommand_Disconnect);
	pCommand->SetOwnerSerial(Serial);

	LogicGatewaySingleton::instance()->PushPacket(pCommand);

	return TRUE;
}

BOOL SFProactorClient::ServiceInitialize( SFProactorService* pService )
{
	int Serial = ProactorServiceMapSingleton::instance()->Register(pService);

	SFASSERT(Serial != INVALID_ID);

	SFPacket* pCommand = PacketPoolSingleton::instance()->Alloc();
	pCommand->SetPacketType(SFCommand_Connect);
	pCommand->SetOwnerSerial(Serial);

	LogicGatewaySingleton::instance()->PushPacket(pCommand);

	pService->SetSerial(Serial);

	return TRUE;
}