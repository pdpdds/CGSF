#include "stdafx.h"
#include "SFNETDispatcher.h"
#include "SFEngine.h"
#include "SFPacket.h"


namespace CgsfNET64Lib { 

	bool SFNETDispatcher::m_bLogicEnd = false;


	SFNETDispatcher::SFNETDispatcher(void)
	{
		m_nLogicThreadCnt = 1;
	}

	SFNETDispatcher::~SFNETDispatcher(void)
	{
	}

	void SFNETDispatcher::Init(int threadCount)
	{
		m_nLogicThreadCnt = threadCount;
	}

	void SFNETDispatcher::Dispatch(BasePacket* pPacket)
	{
		LogicGatewaySingleton::instance()->PushPacket(pPacket);
	}

	void SFNETDispatcher::LogicThreadProc(void* Args)
	{
		UNREFERENCED_PARAMETER(Args);
		
		while (m_bLogicEnd == false)
		{
			//로직게이트웨이 큐에서 패킷을 꺼낸다.
			//로직엔트리 객체의 ProcessPacket 메소드를 호출해서 패킷 처리를 수행한다.
			BasePacket* pPacket = LogicGatewaySingleton::instance()->PopPacket();

			//LOG(INFO) << "LogicThreadProc PacketType: " << pPacket->GetPacketType();
			//google::FlushLogFiles(google::GLOG_INFO);

			LogicEntrySingleton::instance()->ProcessPacket(pPacket);

			ReleasePacket(pPacket);
		}
	}

	bool SFNETDispatcher::ReleasePacket(BasePacket* pPacket)
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
			SFASSERT(0);
			break;

		default:
			SFASSERT(0);
			return false;
		}

		return true;
	}
	
	bool SFNETDispatcher::CreateLogicSystem(ILogicEntry* pLogicEntry)
	{
		ACE_Thread_Manager::instance()->spawn_n(m_nLogicThreadCnt, (ACE_THR_FUNC)LogicThreadProc, this, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY);

		LogicEntrySingleton::instance()->SetLogic(pLogicEntry);

		return true;
	}

	bool SFNETDispatcher::ShutDownLogicSystem()
	{
		m_bLogicEnd = true;

		BasePacket* pCommand = PacketPoolSingleton::instance()->Alloc();
		pCommand->SetSerial(-1);
		pCommand->SetPacketType(SFPACKET_SERVERSHUTDOWN);
		LogicGatewaySingleton::instance()->PushPacket(pCommand);

		return true;
	}

	

}