#include "stdafx.h"
#include "BasePacket.h"
#include "PacketDistribute.h"
#include "WorkerPacket.h"
#include "SubPacketProcess.h"
#include "PktHandlerCommon.h"
#include "Protocol.h"

namespace ServerLib { 
	
	PacketDistribute::PacketDistribute(void)
	{
	}
	
	PacketDistribute::~PacketDistribute(void)
	{
		delete m_pPktHandlerCommon;
	}

	bool PacketDistribute::Initialize()
	{
		return true;
	}

	void PacketDistribute::Stop()
	{
		if (m_IsRunningPacketProcess)
		{
			m_IsRunningPacketProcess = false;
			m_MainPacketThread.join();

			for (int i = 0; i < m_packetWorkerthreadCount; ++i)
			{
				m_pSubPacketProcessList[i]->Stop();
			}
		}
	}

	bool PacketDistribute::ProcessPacket(BasePacket* pPacket)
	{
		if (pPacket == NULL)
		{
			return false;
		}

		WorkerPacket packet;
		packet.Assign(false, pPacket->GetPacketType(), pPacket);
		m_PacketQueues.push(packet);
		return true;
	}
	
	void PacketDistribute::RegistPacketHandler()
	{
		m_pPktHandlerCommon = new PktHandlerCommon;
		m_Dispatch.RegisterMessage((unsigned short)PACKET_ID::SYSTEM_CONNECT, std::bind(&PktHandlerCommon::SystemOnConnect, m_pPktHandlerCommon, std::placeholders::_1));
		m_Dispatch.RegisterMessage((unsigned short)PACKET_ID::SYSTEM_DISCONNECT, std::bind(&PktHandlerCommon::SystemOnDisconnect, m_pPktHandlerCommon, std::placeholders::_1));
		m_Dispatch.RegisterMessage((unsigned short)PACKET_ID::REQUEST_LOGIN, std::bind(&PktHandlerCommon::RequestLoging, m_pPktHandlerCommon, std::placeholders::_1));
	}

	void PacketDistribute::InitPacketWorker(int threadCount)
	{
		RegistPacketHandler();


		m_IsRunningPacketProcess = true;
		m_MainPacketThread = std::thread(&PacketDistribute::PacketProcessLoop, this);
		
		m_packetWorkerthreadCount = threadCount;
		for (int i = 0; i < m_packetWorkerthreadCount; ++i)
		{
			auto process = new SubPacketProcess;
			process->Init();

			m_pSubPacketProcessList.push_back(process);
		}
	}

	int PacketDistribute::GetSubPacketProcessIndex(int sessionID)
	{
		UNREFERENCED_PARAMETER(sessionID);

		return -1;
	}

	void PacketDistribute::PacketProcessLoop()
	{
		WorkerPacket packet;

		while (m_IsRunningPacketProcess)
		{
			if (m_PacketQueues.try_pop(packet) == false)
			{
				Concurrency::wait(0);
				continue;
			}								

			auto sessionID = packet.SessionID();
			auto packetWorkerIndex = -1;

			if (packet.PacketType() == SFPACKET_DATA)
			{
				packetWorkerIndex = GetSubPacketProcessIndex(sessionID);
			}

			if (packetWorkerIndex == -1)
			{
				m_Dispatch.HandleMessage(packet.PacketID(), packet);
				packet.Release();
			}
			else
			{
				m_pSubPacketProcessList[packetWorkerIndex]->InsertPacket(packet);
			}
		}
	}
	

	
}