#include "stdafx.h"
#include "ChatLogicEntry.h"
#include "SFJsonPacket.h"
#include <iostream>
#include "ChatUser.h"

#define CHAT_PACKET_NUM 1000

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
		OnConnectPlayer(pPacket->GetSerial());
		break;

	case SFPACKET_DATA:
		OnPlayerData(pPacket);
		break;

	case SFPACKET_DISCONNECT:	
		OnDisconnectPlayer(pPacket->GetSerial());
		break;

	case SFPACKET_TIMER:
	{
		bool bResult = SFEngine::GetInstance()->CancelTimer(100);
		assert(bResult == true);
	}
		break;

	default:
		return false;
	}

	return true;
}

bool ChatLogicEntry::OnConnectPlayer(int serial)
{
	ChatUser* pUser = new ChatUser();
	pUser->SetSerial(serial);
	pUser->SetName(serial);

	m_ChatUserMap.insert(ChatUserMap::value_type(serial, pUser));

	return true;
}

bool ChatLogicEntry::OnDisconnectPlayer(int serial)
{
	auto& iter = m_ChatUserMap.find(serial);

	if(iter == m_ChatUserMap.end())
	{
		SFASSERT(0);
		return false;
	}

	ChatUser* pUser = iter->second;

	delete pUser;

	m_ChatUserMap.erase(iter);

	return true;
}

bool ChatLogicEntry::OnPlayerData(BasePacket* pPacket)
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;

	switch (pPacket->GetPacketID())
	{
	case CHAT_PACKET_NUM:
		ChatUser* pUser = FindUser(pPacket->GetSerial());

		if (pUser == NULL)
			return false;

		std::cout << pUser->GetName().c_str() << " : " << pJsonPacket->GetData().GetValue<tstring>("chat") << std::endl;

		SFJsonPacket JsonPacket(CHAT_PACKET_NUM);
		JsonObjectNode& ObjectNode = JsonPacket.GetData();
		ObjectNode.Add("who", pUser->GetName());
		ObjectNode.Add("chat", pJsonPacket->GetData().GetValue<tstring>("chat"));

		Broadcast(&JsonPacket);

		break;
	}

	return true;
}

bool ChatLogicEntry::Broadcast(BasePacket* pPacket)
{
	for (auto& chatUser : m_ChatUserMap)
	{
		pPacket->SetSerial(chatUser.first);
		SFEngine::GetInstance()->SendRequest(pPacket);
	}

	return true;
}

ChatUser* ChatLogicEntry::FindUser(int serial)
{
	auto& iter = m_ChatUserMap.find(serial);

	if (iter == m_ChatUserMap.end())
	{
		return NULL;
	}

	return iter->second;
}
