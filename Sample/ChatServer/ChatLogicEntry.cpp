#include "StdAfx.h"
#include "ChatLogicEntry.h"
#include "ChatUser.h"
#include <SFPacketStore/PacketID.h>
#include "SFSinglton.h"
#include <iostream>
#include "SFEngine.h"
#include "SFProtobufPacket.h"
#include <SFPacketStore/ChatPacket.pb.h>

using namespace google;


ChatLogicEntry::ChatLogicEntry(void)
{
}

ChatLogicEntry::~ChatLogicEntry(void)
{
}

BOOL ChatLogicEntry::ProcessPacket(BasePacket* pPacket )
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

BOOL ChatLogicEntry::OnPlayerData( BasePacket* pPacket )
{
	if(pPacket->GetPacketID() == CGSF::ChatReq)
	{
		SFProtobufPacket<ChatPacket::Chat>* pChat = (SFProtobufPacket<ChatPacket::Chat>*)pPacket;
		std::cout << pChat->GetData().chatmessage() << std::endl;

		
		/*SFPacket Packet;
		Packet.SetPacketID(pPacket->GetPacketID());
		Packet.MakePacket(pPacket->GetDataBuffer(), pPacket->GetDataSize(), CGSF_PACKET_OPTION);

		Broadcast(&Packet, pPacket->GetOwnerSerial());*/

		SFProtobufPacket<ChatPacket::Chat> ChatRes(CGSF::ChatRes);
		ChatRes.GetData().set_chatmessage(pChat->GetData().chatmessage());
		Broadcast(&ChatRes);

		return TRUE;
	}

	return FALSE;
}

BOOL ChatLogicEntry::SendRequest(BasePacket* pPacket)
{
	SFEngine::GetInstance()->SendRequest(pPacket);

	return TRUE;
}

BOOL ChatLogicEntry::Broadcast( BasePacket* pPacket)
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