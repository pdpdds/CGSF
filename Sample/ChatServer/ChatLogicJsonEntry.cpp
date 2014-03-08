#include "stdafx.h"
#include "ChatLogicJsonEntry.h"
#include "ChatUser.h"
#include <SFPacketStore/PacketID.h>
#include "SFSinglton.h"
#include <iostream>
#include "SFEngine.h"
#include "SFJsonPacket.h"

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

BOOL ChatLogicJsonEntry::OnConnectPlayer( int Serial )
{
	ChatUser* pUser = new ChatUser();
	pUser->SetSerial(Serial);

	m_ChatUserMap.insert(ChatUserMap::value_type(Serial, pUser));

	return TRUE;
}

BOOL ChatLogicJsonEntry::OnDisconnectPlayer( int Serial )
{
	ChatUserMap::iterator iter = m_ChatUserMap.find(Serial);

	if(iter == m_ChatUserMap.end())
	{
		SFASSERT(0);
		return FALSE;
	}

	ChatUser* pUser = iter->second;

	delete pUser;

	m_ChatUserMap.erase(iter);

	return TRUE;
}

BOOL ChatLogicJsonEntry::OnPlayerData( BasePacket* pPacket )
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;
	int PacketID = pJsonPacket->GetData().GetValue<int>("PacketId");

	if(PacketID == 1234)
	{
		std::cout << pJsonPacket->GetData().GetValue<tstring>("chat") << std::endl;

		
		SFJsonPacket JsonPacket;
		JsonObjectNode& ObjectNode = JsonPacket.GetData();
		ObjectNode.Add("PacketId", 1234);
		ObjectNode.Add("chat", pJsonPacket->GetData().GetValue<tstring>("chat"));

		Broadcast(&JsonPacket);

		return TRUE;
	}

	return FALSE;
}

BOOL ChatLogicJsonEntry::SendRequest(BasePacket* pPacket)
{
	SFEngine::GetInstance()->SendRequest(pPacket);

	return TRUE;
}

BOOL ChatLogicJsonEntry::Broadcast( BasePacket* pPacket)
{
	ChatUserMap::iterator iter = m_ChatUserMap.begin();

	for(;iter != m_ChatUserMap.end(); iter++)
	{
		//if(pPacket->GetOwnerSerial() != iter->first)
		pPacket->SetOwnerSerial(iter->first);	
		SendRequest(pPacket);
	}

	return TRUE;
}