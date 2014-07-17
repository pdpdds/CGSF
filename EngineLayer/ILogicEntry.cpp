#include "stdafx.h"
#include "ILogicEntry.h"
#include "SFEngine.h"
#include "SFServerConnectionManager.h"
#include "EngineInterface\INetworkCallback.h"

bool ILogicEntry::ProcessConnectorPacket(BasePacket* pPacket)
{
	SFServerConnectionManager* pManager = SFEngine::GetInstance()->GetServerConnectionManager();
	auto connector = pManager->GetConnectedServer().find(pPacket->GetSerial());

	if (connector != pManager->GetConnectedServer().end())
	{
		auto& callback = m_mapConnectorCallback.find(connector->second->GetServerInfo().identifer);

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
				pCallback->HandleDisconnect(pPacket->GetSerial());
				break;

			default:
				return false;
			}			
		}
	}
		return true;
}