#include "StdAfx.h"
#include "ChatPacketJsonEntry.h"
#include <SFPacketStore/PacketID.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <SFPacketStore/ChatPacket.pb.h>
#include "SFSinglton.h"
#include <iostream>
#include "SFEngine.h"
#include "SFJsonPacket.h"

//extern SFEngine* g_pEngine;


ChatPacketJsonEntry::ChatPacketJsonEntry(void)
{
}

ChatPacketJsonEntry::~ChatPacketJsonEntry(void)
{
}

BOOL ChatPacketJsonEntry::SendRequest(BasePacket* pPacket)
{
	//g_pEngine->SendRequest(pPacket);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//채팅 서버 예제의 경우는 TCP 처리 쓰레드와 메인 쓰레드가 관섭할 여지가 없기 때문에
//독립적으로 처리를 하고 있습니다.
//TCP 처리 쓰레드에서 받은 패킷을 메인 쓰레드로 넘길 경우
//lock free queue를 사용하면 됩니다.(UDP 쓰레드에서 메인 쓰레드로 패킷 넘기는 부분 참조)

BOOL ChatPacketJsonEntry::ProcessPacket( BasePacket* pPacket)
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

BOOL ChatPacketJsonEntry::OnConnectPlayer( int Serial )
{
	printf("Connect Server!!\n");
	m_Serial = Serial;
	return TRUE;
}

BOOL ChatPacketJsonEntry::OnDisconnectPlayer( int Serial )
{
	printf("Disconnect Server!!\n");

	return TRUE;
}

BOOL ChatPacketJsonEntry::OnPlayerData( BasePacket* pPacket )
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;
	int PacketID = pJsonPacket->GetData().GetValue<int>("PacketId");

	if(PacketID == 1234)
	{
		std::cout << pJsonPacket->GetData().GetValue<tstring>("chat") << std::endl;

		return TRUE;
	}

	return FALSE;
}