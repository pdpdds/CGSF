#include "stdafx.h"

#include "PacketDispatcher.h"
#include "PacketDistribute.h"
#include "ServerNetwork.h"
#include "SFCGSFPacketProtocol.h"


const int PACKET_PROTOCOL_ID = 1;

namespace ServerLib {

	ServerNetwork::ServerNetwork(void)
	{
	}

	ServerNetwork::~ServerNetwork(void)
	{
	}

	NET_ERROR_CODE ServerNetwork::Init(int dispatchThreadCount, int maxBufferSize, int maxPacketSize, int packetOption)
	{
		m_pPacketDispatcher = std::unique_ptr<PacketDispatcher>(new PacketDispatcher);
		m_pPacketDispatcher->Init(dispatchThreadCount);

		m_pPacketDistribute = std::unique_ptr<PacketDistribute>(new PacketDistribute);
		m_pPacketDistribute->InitPacketWorker(4);

		auto initResult = SFEngine::GetInstance()->Intialize(m_pPacketDistribute.get(), m_pPacketDispatcher.get());
		if (initResult != NET_ERROR_CODE::SUCCESS)
		{
			return initResult;
		}

		auto packetProtocol = new SFPacketProtocol<SFCGSFPacketProtocol>(maxBufferSize, maxPacketSize, packetOption);
		if (SFEngine::GetInstance()->AddPacketProtocol(PACKET_PROTOCOL_ID, packetProtocol) == false)
		{
			return NET_ERROR_CODE::ENGINE_INIT_ADD_PACKET_PROTOCOL_FAIL;
		}

		return NET_ERROR_CODE::SUCCESS;
	}

	bool ServerNetwork::Start()
	{
		SFEngine::GetInstance()->Start(PACKET_PROTOCOL_ID);
		return true;
	}

	void ServerNetwork::Stop()
	{
		SFEngine::GetInstance()->ShutDown();

		m_pPacketDistribute->Stop();
	}

	

}