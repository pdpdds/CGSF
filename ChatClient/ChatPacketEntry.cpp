#include "StdAfx.h"
#include "ChatPacketEntry.h"
#include "SFPacket.h"
#include "PacketID.h"
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "ChatPacket.pb.h"
#include "SFSinglton.h"
#include <iostream>
#include "SFEngine.h"
#include "SFClient.h"
#include "GoogleLog.h"
#include "SFProactorClient.h"
#include "SFProactorService.h"
#include "SFMGFramework.h"

extern SFSYSTEM_CLIENT* g_pNetworkEngine;

ChatPacketEntry::ChatPacketEntry(void)
{
}

ChatPacketEntry::~ChatPacketEntry(void)
{
}

BOOL ChatPacketEntry::Send( int Serial, SFPacket* pPacket )
{
	g_pNetworkEngine->GetNetworkPolicy()->Send(Serial, pPacket);

	return TRUE;
}

BOOL ChatPacketEntry::Send(USHORT PacketID, char* pBuffer, int BufSize )
{
	SFPacket Packet;
	Packet.SetPacketID(PacketID);
	Packet.MakePacket((BYTE*)pBuffer, BufSize, CGSF_PACKET_OPTION);

	g_pNetworkEngine->GetNetworkPolicy()->Send(GetSerial(), &Packet);

	return TRUE;
}

BOOL ChatPacketEntry::Send(int Serial, USHORT PacketID, char* pBuffer, int BufSize )
{
	g_pNetworkEngine->GetNetworkPolicy()->Send(Serial, PacketID, pBuffer, BufSize);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//채팅 서버 예제의 경우는 TCP 처리 쓰레드와 메인 쓰레드가 관섭할 여지가 없기 때문에
//독립적으로 처리를 하고 있습니다.
//TCP 처리 쓰레드에서 받은 패킷을 메인 쓰레드로 넘길 경우
//lock free queue를 사용하면 됩니다.(UDP 쓰레드에서 메인 쓰레드로 패킷 넘기는 부분 참조)

BOOL ChatPacketEntry::ProcessPacket( SFCommand* pCommand )
{
	SFPacket* pPacket = (SFPacket*)(pCommand);
	if(pPacket->GetPacketID() == CGSF::ChatReq)
	{
		ChatPacket::Chat PktChat;
		protobuf::io::ArrayInputStream is(pPacket->GetDataBuffer(), pPacket->GetDataSize());
		PktChat.ParseFromZeroCopyStream(&is);

		std::cout << PktChat.chatmessage() << std::endl;

		return TRUE;
	}
	else if(pCommand->GetPacketType() == SFCommand_Disconnect)
	{
		g_pNetworkEngine->SetProcessing(FALSE);
		printf("Disconnect Server!!\n");
	}
	else if(pCommand->GetPacketType() == SFCommand_Connect)
	{
		g_pNetworkEngine->SetProcessing(TRUE);
		printf("Connect Server!!\n");

		m_Serial = pCommand->GetOwnerSerial();
	}

	return FALSE;

}