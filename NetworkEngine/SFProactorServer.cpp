#include "StdAfx.h"
#include "SFProactorServer.h"
#include "PacketCore.pb.h"
#include "SFBridgeThread.h"

extern BOOL gServerEnd;

BOOL SendAuthPacket(SFProactorService* pService)
{
	PacketCore::Auth PktAuth;
	PktAuth.set_encryptionkey(ENCRYPTION_KEY);

	int BufSize = PktAuth.ByteSize();

	char Buffer[2048] = {0,};

	if(BufSize != 0)
	{
		::google::protobuf::io::ArrayOutputStream os(Buffer, BufSize);
		PktAuth.SerializeToZeroCopyStream(&os);
	}

	pService->Send(CGSF::Auth, Buffer, BufSize);

	return TRUE;
}

SFProactorServer::SFProactorServer(void)
: m_Acceptor(this)
{
	//ACEAllocator* AceMemoryPool = new ACEAllocator(100, sizeof(SFProactorService));
}

SFProactorServer::~SFProactorServer(void)
{

}

BOOL SFProactorServer::Run()
{
	ACE_INET_Addr listen_addr;

	listen_addr.set(25251);

	if(0 != m_Acceptor.open(listen_addr, 0, 1, ACE_DEFAULT_BACKLOG, 1, 0, 1, 1, 1024))
		return FALSE;

	return TRUE;
}

BOOL SFProactorServer::ServiceDisconnect( int Serial )
{
	ProactorServiceMapSingleton::instance()->UnRegister(Serial);

	SFPacket* pCommand = PacketPoolSingleton::instance()->Alloc();
	pCommand->SetPacketType(SFCommand_Disconnect);
	pCommand->SetOwnerSerial(Serial);

	LogicGatewaySingleton::instance()->PushPacket(pCommand);

	return TRUE;
}

BOOL SFProactorServer::ServiceInitialize(SFProactorService* pService)
{
	int Serial = ProactorServiceMapSingleton::instance()->Register(pService);

	SFASSERT(Serial != INVALID_ID);

	SFPacket* pCommand = PacketPoolSingleton::instance()->Alloc();
	pCommand->SetPacketType(SFCommand_Connect);
	pCommand->SetOwnerSerial(Serial);

	LogicGatewaySingleton::instance()->PushPacket(pCommand);

	pService->SetSerial(Serial);

	SendAuthPacket(pService);

	return TRUE;
}

BOOL SFProactorServer::Start( ILogicEntry* pLogic )
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	int OptimalThreadCount =si.dwNumberOfProcessors * 2;

	m_WorkThreadGroupID = ACE_Thread_Manager::instance()->spawn_n(OptimalThreadCount, (ACE_THR_FUNC)ProactiveWorkThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1);

	if(m_WorkThreadGroupID == -1)
	{
		SFASSERT(0);
		return FALSE;
	}

	if(pLogic != NULL)
	{
		m_LogicThreadGroupID = ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)BusinessThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2) == -1;

		if(m_LogicThreadGroupID == -1)
		{
			SFASSERT(0);
			return FALSE;
		}

		LogicEntrySingleton::instance()->SetLogic(pLogic);

		CreateTimerTask(pLogic);
	}

	return Run();
}

BOOL SFProactorServer::End()
{
	gServerEnd = TRUE;

	ACE_Thread_Manager::instance()->wait_grp(m_WorkThreadGroupID);

	ACE_Thread_Manager::instance()->wait_grp(m_LogicThreadGroupID);

	return TRUE;
}

BOOL SFProactorServer::AddTimer(DWORD TimerID, DWORD StartTime, DWORD Period)
{
	  ACE_Time_Value Interval(Period/1000, (Period%1000)*1000);
	  ACE_Time_Value Start(StartTime/1000, (StartTime%1000)*1000);

  if (ACE_Proactor::instance ()->schedule_timer (m_TimeOutHandler,
                                                 (void*)TimerID,
                                                 Start,
                                                 Interval) == -1)
    return FALSE;

	return TRUE;
}

BOOL SFProactorServer::CreateTimerTask( ILogicEntry* pLogic)
{
	ILogicEntry::TimerMap TimerMap = pLogic->GetTimerMap();

	ILogicEntry::TimerMap::iterator iter = TimerMap.begin();

	for(;iter != TimerMap.end(); iter++)
	{
		AddTimer(iter->first, iter->second.StartDelay, iter->second.Period);
	}

	return TRUE;
}

BOOL SFProactorServer::Send(int Serial, USHORT PacketID, char* pMessage, int BufSize )
{
	return ProactorServiceMapSingleton::instance()->Send(Serial, PacketID, pMessage, BufSize );
}

BOOL SFProactorServer::Send(int Serial, SFPacket* pPacket)
{
	return ProactorServiceMapSingleton::instance()->Send(Serial, pPacket);
}