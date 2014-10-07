#include "stdafx.h"

#include "SubPacketProcess.h"
#include "PktHandlerLobby.h"
#include "Protocol.h"

namespace ServerLib {

	SubPacketProcess::SubPacketProcess()
	{
	}

	SubPacketProcess::~SubPacketProcess()
	{
		delete m_pPktHandlerLobby;
	}

	void SubPacketProcess::Stop()
	{
		if (m_IsRunningPacketProcess)
		{
			m_IsRunningPacketProcess = false;

			m_PacketThread.join();
		}
	}

	void SubPacketProcess::Init()
	{
		RegistPacketHandler();

		m_IsRunningPacketProcess = true;
		m_PacketThread = std::thread(&SubPacketProcess::PacketProcessLoop, this);
	}

	void SubPacketProcess::RegistPacketHandler()
	{
		m_pPktHandlerLobby = new PktHandlerLobby;

		m_Dispatch.RegisterMessage((unsigned short)PACKET_ID::REQUEST_LOGIN, std::bind(&PktHandlerLobby::RequestLeaveLobby, m_pPktHandlerLobby, std::placeholders::_1));
	}

	void SubPacketProcess::InsertPacket(WorkerPacket& packet)
	{
		m_PacketQueues.push(packet);
	}
	
	void SubPacketProcess::PacketProcessLoop()
	{
		WorkerPacket packet;

		while (m_IsRunningPacketProcess)
		{
			if (m_PacketQueues.try_pop(packet) == false)
			{
				Concurrency::wait(0);
				continue;
			}

			
			m_Dispatch.HandleMessage(packet.PacketID(), packet);
			packet.Release();
		}
	}
}