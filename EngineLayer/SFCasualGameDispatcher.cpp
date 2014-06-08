#include "stdafx.h"
#include "SFCasualGameDispatcher.h"
#include "SFEngine.h"
#include "SFDatabase.h"
#include "IRPCInterface.h"
#include "SFPacket.h"
#include "SFEngine.h"

//로직 쓰레드 수행 메소드 설정 및 로직쓰레드의 개수 설정
SFCasualGameDispatcher::SFCasualGameDispatcher(void)
{
	m_nLogicThreadCnt = 1;
	m_funcBusnessThread = (void*)SFCasualGameDispatcher::BusinessThread;
	m_funcRPCThread = (void*)SFCasualGameDispatcher::RPCThread;
}

SFCasualGameDispatcher::~SFCasualGameDispatcher(void)
{
}

//로직게이트웨이 큐에 패킷을 큐잉한다.
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

void SFCasualGameDispatcher::BusinessThread(void* Args)
{
	SFEngine* pEngine = (SFEngine*)Args;
	while (SFEngine::GetInstance()->ServerTerminated() == FALSE)
	{
//로직게이트웨이 큐에서 패킷을 꺼낸다.
		BasePacket* pPacket = LogicGatewaySingleton::instance()->PopPacket();
//로직엔트리 객체의 ProcessPacket 메소드를 호출해서 패킷 처리를 수행한다.
		LogicEntrySingleton::instance()->ProcessPacket(pPacket);

//사용한 패킷을 수거한다. 패킷의 타입에 따라 릴리즈 형태가 다름
		switch (pPacket->GetPacketType())
		{
		case SFPACKET_DATA:
			pEngine->ReleasePacket(pPacket);
			break;
		case SFPACKET_CONNECT:
		case SFPACKET_DISCONNECT:
		case SFPACKET_TIMER:
		case SFPACKET_SHOUTER:
			delete pPacket;
			break;

		case SFPACKET_DB:
			SFDatabase::RecallDBMsg((SFMessage*)pPacket);
			break;

		case SFPACKET_SERVERSHUTDOWN:
			return;

		default:
			SFASSERT(0);
		}
	}
}

void SFCasualGameDispatcher::RPCThread(void* Args)
{
	SFEngine* pEngine = (SFEngine*)Args;
	IRPCInterface* pRPC = pEngine->GetRPCManager();


	while (SFEngine::GetInstance()->ServerTerminated() == FALSE)
	{
		//로직게이트웨이 큐에서 패킷을 꺼낸다.
		//BasePacket* pPacket = RPCGatewaySingleton::instance()->PopPacket();

//20140608 임시..
		SFPacket* pPacket = (SFPacket*)RPCGatewaySingleton::instance()->PopPacket();
	
		pRPC->ProcessRPCService(pPacket);

		delete pPacket;
	}
}


