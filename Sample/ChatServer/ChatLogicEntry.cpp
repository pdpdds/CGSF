#include "stdafx.h"
#include "ChatLogicEntry.h"
#include "SFJsonPacket.h"
#include <iostream>
#include "ChatUser.h"

#define CHAT_PACKET_NUM 0x1234

ChatLogicEntry::ChatLogicEntry(void)
{
}


ChatLogicEntry::~ChatLogicEntry(void)
{
}

bool ChatLogicEntry::Initialize()
{
	return true;
}

bool ChatLogicEntry::ProcessPacket(BasePacket* pPacket)
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

bool ChatLogicEntry::OnConnectPlayer(int Serial)
{
	ChatUser* pUser = new ChatUser();
	pUser->SetSerial(Serial);

	m_ChatUserMap.insert(ChatUserMap::value_type(Serial, pUser));

	return TRUE;
}

bool ChatLogicEntry::OnDisconnectPlayer(int Serial)
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

bool ChatLogicEntry::OnPlayerData(BasePacket* pPacket)
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;

	switch (pPacket->GetPacketID())
	{
	case CHAT_PACKET_NUM:
	{
				 std::cout << pJsonPacket->GetData().GetValue<tstring>("chat") << std::endl;

				 SFJsonPacket JsonPacket(CHAT_PACKET_NUM);
				 JsonObjectNode& ObjectNode = JsonPacket.GetData();
				 ObjectNode.Add("chat", pJsonPacket->GetData().GetValue<tstring>("chat"));

				 Broadcast(&JsonPacket);				
	}
		break;
	}

	return true;
}

bool ChatLogicEntry::Broadcast(BasePacket* pPacket)
{

	for (auto& chatUser : m_ChatUserMap)
	{
		pPacket->SetOwnerSerial(chatUser.first);
		SFEngine::GetInstance()->SendRequest(pPacket);
	}

	return true;
}