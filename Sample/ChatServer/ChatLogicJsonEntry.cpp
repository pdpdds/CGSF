#include "stdafx.h"
#include "ChatLogicJsonEntry.h"
#include "SFJsonPacket.h"
#include <iostream>
#include "ChatUser.h"

#define CHAT_PACKET_NUM 0x1234

ChatLogicJsonEntry::ChatLogicJsonEntry(void)
{
}


ChatLogicJsonEntry::~ChatLogicJsonEntry(void)
{
}

bool ChatLogicJsonEntry::Initialize()
{
	return true;
}

bool ChatLogicJsonEntry::ProcessPacket(BasePacket* pPacket)
{
	switch (pPacket->GetPacketType())
	{
	case SFPACKET_CONNECT:
		{	
			OnConnectPlayer(pPacket->GetOwnerSerial());
		}
		break;

	case SFPACKET_DATA:
		{	
			OnPlayerData(pPacket);
		}
		break;

	case SFPACKET_DISCONNECT:
		{	
			OnDisconnectPlayer(pPacket->GetOwnerSerial());
		}
		break;

	case SFPACKET_TIMER:
		{	
			//OnTimer(pCommand->GetOwnerSerial());
		}
		break;

	default:
		return false;
	}

	return true;
}

bool ChatLogicJsonEntry::OnConnectPlayer( int Serial )
{
	ChatUser* pUser = new ChatUser();
	pUser->SetSerial(Serial);

	m_ChatUserMap.insert(ChatUserMap::value_type(Serial, pUser));

	return TRUE;
}

bool ChatLogicJsonEntry::OnDisconnectPlayer( int Serial )
{
	auto& iter = m_ChatUserMap.find(Serial);

	if(iter == m_ChatUserMap.end())
	{
		SFASSERT(0);
		return false;
	}

	ChatUser* pUser = iter->second;

	delete pUser;

	m_ChatUserMap.erase(iter);

	return TRUE;
}

bool ChatLogicJsonEntry::OnPlayerData( BasePacket* pPacket )
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;
	int PacketID = pJsonPacket->GetData().GetValue<int>("PacketId");

	switch (pJsonPacket->GetData().GetValue<int>("PacketId"))
	{
	case CHAT_PACKET_NUM:
	{
				 std::cout << pJsonPacket->GetData().GetValue<tstring>("chat") << std::endl;

				 SFJsonPacket JsonPacket;
				 JsonObjectNode& ObjectNode = JsonPacket.GetData();
				 ObjectNode.Add("PacketId", CHAT_PACKET_NUM);
				 ObjectNode.Add("chat", pJsonPacket->GetData().GetValue<tstring>("chat"));

				 Broadcast(&JsonPacket);				
	}
		break;
	}

	return true;
}

bool ChatLogicJsonEntry::Broadcast(BasePacket* pPacket)
{

	for (auto& chatUser : m_ChatUserMap)
	{
		pPacket->SetOwnerSerial(chatUser.first);
		SFEngine::GetInstance()->SendRequest(pPacket);
	}

	return true;
}