#include "stdafx.h"
#include "SFCasualGameDispatcher.h"
#include "SFEngine.h"
#include "SFDatabase.h"
#include "IRPCService.h"
#include "SFPacket.h"

bool SFCasualGameDispatcher::m_bLogicEnd = false;


SFCasualGameDispatcher::SFCasualGameDispatcher(void)
{
//캐쥬얼 게임 프레임 워크의 로직 쓰레드 수는 하나임
	m_nLogicThreadCnt = 1;
}

SFCasualGameDispatcher::~SFCasualGameDispatcher(void)
{
}

void SFCasualGameDispatcher::Dispatch(BasePacket* pPacket)
{
	if (pPacket->GetPacketType() == SFPACKET_RPC && SFEngine::GetInstance()->IsServer())
	{
		RPCGatewaySingleton::instance()->PushPacket(pPacket);
	}
	else
	{
		LogicGatewaySingleton::instance()->PushPacket(pPacket);
	}
}

void SFCasualGameDispatcher::LogicThreadProc(void* Args)
{
	UNREFERENCED_PARAMETER(Args);

	SFEngine* pEngine = SFEngine::GetInstance();

	while (m_bLogicEnd == false)
	{
//로직게이트웨이 큐에서 패킷을 꺼낸다.
//로직엔트리 객체의 ProcessPacket 메소드를 호출해서 패킷 처리를 수행한다.
		BasePacket* pPacket = LogicGatewaySingleton::instance()->PopPacket();
		LogicEntrySingleton::instance()->ProcessPacket(pPacket);

		ReleasePacket(pPacket);
	}
}

bool SFCasualGameDispatcher::ReleasePacket(BasePacket* pPacket)
{
	//사용한 패킷을 수거한다. 패킷의 타입에 따라 릴리즈 형태가 다름
	switch (pPacket->GetPacketType())
	{
	case SFPACKET_DATA:
		SFEngine::GetInstance()->ReleasePacket(pPacket);
		break;
	case SFPACKET_CONNECT:
	case SFPACKET_DISCONNECT:
	case SFPACKET_TIMER:
	case SFPACKET_SHOUTER:
	case SFPACKET_SERVERSHUTDOWN:
		delete pPacket;
		break;

	case SFPACKET_DB:
		SFDatabase::RecallDBMsg((SFMessage*)pPacket);
		break;

	default:
		SFASSERT(0);
		return false;
	}

	return true;
}

void SFCasualGameDispatcher::RPCThreadProc(void* Args)
{
	SFCasualGameDispatcher* pDisPatcher = (SFCasualGameDispatcher*)Args;
	while (m_bLogicEnd == false)
	{
		//로직게이트웨이 큐에서 패킷을 꺼낸다.
		//BasePacket* pPacket = RPCGatewaySingleton::instance()->PopPacket();

//20140608 임시..
		SFPacket* pPacket = (SFPacket*)RPCGatewaySingleton::instance()->PopPacket();
	
		pDisPatcher->m_pRPCService->ProcessRPCService(pPacket);

		delete pPacket;
	}
}

bool SFCasualGameDispatcher::CreateLogicSystem(ILogicEntry* pLogicEntry)
{	
	ACE_Thread_Manager::instance()->spawn_n(m_nLogicThreadCnt, (ACE_THR_FUNC)LogicThreadProc, this, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY);

	LogicEntrySingleton::instance()->SetLogic(pLogicEntry);

	ACE_Thread_Manager::instance()->spawn_n(4, (ACE_THR_FUNC)RPCThreadProc, this, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY);

	return true;
}

bool SFCasualGameDispatcher::ShutDownLogicSystem()
{
	m_bLogicEnd = true;

	BasePacket* pCommand = PacketPoolSingleton::instance()->Alloc();
	pCommand->SetSerial(-1);
	pCommand->SetPacketType(SFPACKET_SERVERSHUTDOWN);
	LogicGatewaySingleton::instance()->PushPacket(pCommand);

	return true;
}

bool SFCasualGameDispatcher::AddRPCService(IRPCService* pService)
{
	m_pRPCService = pService;
	return true;
}

