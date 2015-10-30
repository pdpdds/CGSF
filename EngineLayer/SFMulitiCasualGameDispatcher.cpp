#include "stdafx.h"
#include "SFMulitiCasualGameDispatcher.h"
#include "SFEngine.h"
#include "EngineInterface/ISession.h"

bool SFMulitiCasualGameDispatcher::m_bLogicEnd = false;

typedef struct tag_CasualGameParam
{
	SFIOCPQueue<BasePacket>* pQueue;
	ILogicEntry* pLogicEntry;

}CasualGameParam;

SFMulitiCasualGameDispatcher::SFMulitiCasualGameDispatcher(int channelCount)
: m_channelCount(channelCount)
{

}

SFMulitiCasualGameDispatcher::~SFMulitiCasualGameDispatcher()
{
}

void SFMulitiCasualGameDispatcher::Dispatch(BasePacket* pPacket)
{		
	ISession* pSession = pPacket->GetSessionDesc().pSession;

	switch (pPacket->GetPacketType())
	{
	case SFPACKET_CONNECT:
	{
//#ifdef _DEBUG
	/*	OnAuthenticate(pPacket);
		pSession->SetLoginState(SESSION_STATE_AUTENTICATE);
		pSession->m_channelNum = 1;
		SFPacket* pkt = PacketPoolSingleton::instance()->Alloc();
		pkt->SetPacketType(SFPACKET_DATA);
		pkt->SetSerial(pPacket->GetSerial());
		pkt->SetPacketID(12345);
		pkt->SetSessionDesc(pPacket->GetSessionDesc());

		DistributePacket(pkt);*/
//#else
		pSession->SetLoginState(SESSION_STATE_CONNECT);		
//#endif
		ReleasePacket(pPacket);
	}
		return;
	case SFPACKET_DISCONNECT:
	{
		if (pSession->GetLoginState() == SESSION_STATE_AUTENTICATE)
		{
			DistributePacket(pPacket);
		}
		
		return;
	}
	case SFPACKET_DATA:
		if (pSession->GetLoginState() == SESSION_STATE_AUTENTICATE)
		{
			DistributePacket(pPacket);
		}
		else if (pSession->GetLoginState() == SESSION_STATE_CONNECT)
		{
			if (true == OnAuthenticate(pPacket))
			{
				pSession->m_channelNum = 1;
				pSession->SetLoginState(SESSION_STATE_AUTENTICATE);
				DistributePacket(pPacket);
			}
			else
			{
				pSession->SetLoginState(SESSION_STATE_NULL);
				SFEngine::GetInstance()->Disconnect(pPacket->GetSerial());
				ReleasePacket(pPacket);
			}
		}
		else
		{
			pSession->SetLoginState(SESSION_STATE_NULL);
			SFEngine::GetInstance()->Disconnect(pPacket->GetSerial());

			ReleasePacket(pPacket);
		}
	}
}

bool SFMulitiCasualGameDispatcher::DistributePacket(BasePacket* pPacket)
{
	ISession* pSession = pPacket->GetSessionDesc().pSession;
	int channelNum = pSession->m_channelNum;

	if (channelNum < 0 || channelNum >= m_vecQueue.size())
		return false;
	
	m_vecQueue[channelNum]->Push(pPacket);

	return true;
}

bool SFMulitiCasualGameDispatcher::CreateLogicSystem(ILogicEntry* pLogicEntry)
{
	LogicEntrySingleton::instance()->SetLogic(pLogicEntry);
	
	for (int index = 0; index < m_channelCount; index++)
	{
		CasualGameParam* pParam = new CasualGameParam();
		SFIOCPQueue<BasePacket>* pQueue = new SFIOCPQueue<BasePacket>();
		pParam->pQueue = pQueue;
		pParam->pLogicEntry = pLogicEntry->Clone();
		pParam->pLogicEntry->Initialize();


		ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)CasualGameLogicProc, pParam, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY);

		m_vecQueue.push_back(pQueue);		
	}	
	
	return true;
}

bool SFMulitiCasualGameDispatcher::ShutDownLogicSystem()
{
	return true;
}

void SFMulitiCasualGameDispatcher::CasualGameLogicProc(void* Args)
{
	CasualGameParam* pParam = static_cast<CasualGameParam*>(Args);
	SFIOCPQueue<BasePacket>* pQueue = pParam->pQueue;
	ILogicEntry* pEntry = pParam->pLogicEntry;

	while (m_bLogicEnd == false)
	{
		BasePacket* pPacket = pQueue->Pop(INFINITE);

		if (pPacket)
		{
			pEntry->ProcessPacket(pPacket);
			if (pPacket->GetPacketType() != SFPACKET_DB)
			{
				ReleasePacket(pPacket);
			}
		}
	}
}