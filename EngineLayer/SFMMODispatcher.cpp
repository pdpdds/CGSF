#include "stdafx.h"
#include "SFMMODispatcher.h"
#include "SFEngine.h"
#include "SFDatabase.h"

bool SFMMODispatcher::m_bLogicEnd = false;

SFMMODispatcher::SFMMODispatcher(int channelCount)
: m_channelCount(channelCount)
{

}


SFMMODispatcher::~SFMMODispatcher()
{
}

void SFMMODispatcher::Dispatch(BasePacket* pPacket)
{
	LogicGatewaySingleton::instance()->PushPacket(pPacket);
}

bool SFMMODispatcher::CreateLogicSystem(ILogicEntry* pLogicEntry)
{
	UNREFERENCED_PARAMETER(pLogicEntry);

	ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)PacketDistributorProc, this, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY);

	for (int index = 0; m_channelCount; index++)
	{
		SFIOCPQueue<BasePacket>* pQueue = new SFIOCPQueue<BasePacket>();
		ACE_thread_t handle = 0;
		ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)MMOLogicProc, pQueue, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, handle);

		m_mapQueue.insert(std::make_pair(index, pQueue));
	}
	
	return true;
}

bool SFMMODispatcher::ShutDownLogicSystem()
{
	return true;
}

void SFMMODispatcher::PacketDistributorProc(void* Args)
{
	SFMMODispatcher* pMMODispatcher = static_cast<SFMMODispatcher*>(Args);

	//SFEngine* pEngine = SFEngine::GetInstance();

	while (m_bLogicEnd == false)
	{
		BasePacket* pPacket = LogicGatewaySingleton::instance()->PopPacket();
		
		int channelNum = 0;

		const auto& iter = pMMODispatcher->m_mapQueue.find(channelNum);

		if (iter == pMMODispatcher->m_mapQueue.end())
		{
			SFASSERT(0);
			ReleasePacket(pPacket);
			continue;
		}

		SFIOCPQueue<BasePacket>* pQueue = iter->second;
		pQueue->Push(pPacket);
	}
}

void SFMMODispatcher::MMOLogicProc(void* Args)
{
	SFIOCPQueue<BasePacket>* pQueue = static_cast<SFIOCPQueue<BasePacket>*>(Args);

	while (m_bLogicEnd)
	{
		BasePacket* pPacket = pQueue->Pop(INFINITE);

		if (pPacket)
		{
			LogicEntrySingleton::instance()->ProcessPacket(pPacket);
			ReleasePacket(pPacket);
		}
	}
}

bool SFMMODispatcher::ReleasePacket(BasePacket* pPacket)
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