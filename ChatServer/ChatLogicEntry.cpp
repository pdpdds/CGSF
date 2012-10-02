#include "StdAfx.h"
#include "ChatLogicEntry.h"
#include "SFPacket.h"
#include "ChatUser.h"
#include "PacketID.h"
#include "SFSinglton.h"
#include <iostream>
#include "SFEngine.h"
#include "SFServer.h"
#include "GoogleLog.h"
#include "SFMGFramework.h"

using namespace google;

extern SFSYSTEM_SERVER* g_pEngine;

ChatLogicEntry::ChatLogicEntry(void)
{
}

ChatLogicEntry::~ChatLogicEntry(void)
{
}

BOOL ChatLogicEntry::ProcessPacket( SFCommand* pCommand )
{
	switch (pCommand->GetPacketType())
	{
	case SFCommand_Connect:
		{	
			OnConnectPlayer(pCommand->GetOwnerSerial());
		}
		break;

	case SFCommand_Data:
		{	
			OnPlayerData((SFPacket*)pCommand);
		}
		break;

	case SFCommand_Disconnect:
		{	
			OnDisconnectPlayer(pCommand->GetOwnerSerial());
		}
		break;

	case SFCommand_Timer:
		{	
			//OnTimer(pCommand->GetOwnerSerial());
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

BOOL ChatLogicEntry::OnConnectPlayer( int Serial )
{
	ChatUser* pUser = new ChatUser();
	pUser->SetSerial(Serial);

	m_ChatUserMap.insert(ChatUserMap::value_type(Serial, pUser));

	return TRUE;
}

BOOL ChatLogicEntry::OnDisconnectPlayer( int Serial )
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

BOOL ChatLogicEntry::OnPlayerData( SFPacket* pPacket )
{
	if(pPacket->GetPacketID() == CGSF::ChatReq)
	{
		ChatPacket::Chat PktChat;
		protobuf::io::ArrayInputStream is(pPacket->GetDataBuffer(), pPacket->GetDataSize());
		PktChat.ParseFromZeroCopyStream(&is);
		
		std::cout << PktChat.chatmessage() << std::endl;

		SFPacket Packet;
		Packet.SetPacketID(pPacket->GetPacketID());
		Packet.MakePacket(pPacket->GetDataBuffer(), pPacket->GetDataSize(), CGSF_PACKET_OPTION);

		Broadcast(&Packet, pPacket->GetOwnerSerial());

		return TRUE;
	}

	return FALSE;
}

BOOL ChatLogicEntry::Send(int Serial, SFPacket* pPacket)
{
	g_pEngine->GetNetworkPolicy()->Send(Serial, pPacket);

	return TRUE;
}

BOOL ChatLogicEntry::Broadcast( SFPacket* pPacket, int Serial)
{
	ChatUserMap::iterator iter = m_ChatUserMap.begin();

	for(;iter != m_ChatUserMap.end(); iter++)
	{
		if(Serial != iter->first)
			Send(iter->first, pPacket);
	}

	return TRUE;
}