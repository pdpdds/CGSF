#pragma once 

#include <concurrent_queue.h>
#include <thread>

#include "SFDispatch.h"

#include "WorkerPacket.h"



namespace ServerLib {
	class PktHandlerLobby;

	class SubPacketProcess
	{
	public:
		SubPacketProcess();
		~SubPacketProcess();

		void Init();

		void InsertPacket(WorkerPacket& packet);

		void Stop();

	private:
		void RegistPacketHandler();

		void PacketProcessLoop();


		PktHandlerLobby* m_pPktHandlerLobby = nullptr;

		SFDispatch<USHORT, std::function<short(WorkerPacket&)>, WorkerPacket&> m_Dispatch;


		Concurrency::concurrent_queue<WorkerPacket> m_PacketQueues;

		bool m_IsRunningPacketProcess = false;
		std::thread m_PacketThread;
	};
}