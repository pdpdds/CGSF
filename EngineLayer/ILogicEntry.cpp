#include "stdafx.h"
#include "ILogicEntry.h"
#include "SFEngine.h"
#include "SFServerConnectionManager.h"
#include "EngineInterface\INetworkCallback.h"
#include "SFPacketProtocolManager.h"

bool ILogicEntry::ProcessConnectorPacket(BasePacket* pPacket)
{
	_SessionDesc& desc = pPacket->GetSessionDesc();

	auto& callback = m_mapConnectorCallback.find(desc.identifier);

	if (callback != m_mapConnectorCallback.end())
	{
		INetworkCallback* pCallback = callback->second;
		switch (pPacket->GetPacketType())
		{

		case SFPACKET_DATA:
			pCallback->HandleNetworkMessage(pPacket);
			break;

		case SFPACKET_CONNECT:
			pCallback->HandleConnect(pPacket->GetSerial());
			break;

		case  SFPACKET_DISCONNECT:
		{
									 pCallback->HandleDisconnect(pPacket->GetSerial());
									 SFServerConnectionManager* pManager = SFEngine::GetInstance()->GetServerConnectionManager();
									 pManager->SetConnectorState(desc.identifier, false);
		}
			break;

		default:
			return false;
		}
	}

	return true;
}

bool ILogicEntry::AddConnectorCallback(int identifier, INetworkCallback* pCallback, int packetProtocolId)
{
	SFPacketProtocolManager* pManager = SFEngine::GetInstance()->GetPacketProtocolManager();
	if (false == pManager->SetConnectorPacketProtocol(identifier, packetProtocolId))
		return false;

	m_mapConnectorCallback.insert(std::make_pair(identifier, pCallback)); 
	return true;
}