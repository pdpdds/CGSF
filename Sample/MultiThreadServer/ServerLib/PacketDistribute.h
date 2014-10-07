#pragma once

#include <concurrent_queue.h>
#include <unordered_map>
#include <thread>

#include "ILogicEntry.h"
#include "SFDispatch.h"

#include "WorkerPacket.h"


namespace ServerLib { 
	
	class PktHandlerCommon;
	class SubPacketProcess;


	class PacketDistribute : public ILogicEntry
	{
	public:
		PacketDistribute(void);
		virtual ~PacketDistribute(void);

		virtual bool Initialize() override;
		virtual bool ProcessPacket(BasePacket* pBasePacket) override;
		

		void InitPacketWorker(int threadCount);

		void Stop();
		


	private:	

		void RegistPacketHandler();

		void PacketProcessLoop();
		
		int GetSubPacketProcessIndex(int sessionID);
		

				
		PktHandlerCommon* m_pPktHandlerCommon = nullptr;

		SFDispatch<USHORT, std::function<short(WorkerPacket&)>, WorkerPacket&> m_Dispatch;
		
		Concurrency::concurrent_queue<WorkerPacket> m_PacketQueues;
		

		bool m_IsRunningPacketProcess = false;
		std::thread m_MainPacketThread;

		int m_packetWorkerthreadCount = 1;
		std::vector<SubPacketProcess*> m_pSubPacketProcessList;
	};

}