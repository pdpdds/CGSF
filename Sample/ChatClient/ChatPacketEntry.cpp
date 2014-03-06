#include "StdAfx.h"
#include "ChatPacketEntry.h"
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <SFPacketStore/PacketID.h>
#include <SFPacketStore/ChatPacket.pb.h>
#include "SFSinglton.h"
#include <iostream>
#include "SFEngine.h"
#include "SFProtobufPacket.h"

//extern SFEngine* g_pEngine;

using namespace google;

ChatPacketEntry::ChatPacketEntry(void)
{
}

ChatPacketEntry::~ChatPacketEntry(void)
{
}

BOOL ChatPacketEntry::SendRequest(BasePacket* pPacket)
{
	//g_pEngine->SendRequest(pPacket);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//채팅 서버 예제의 경우는 TCP 처리 쓰레드와 메인 쓰레드가 관섭할 여지가 없기 때문에
//독립적으로 처리를 하고 있습니다.
//TCP 처리 쓰레드에서 받은 패킷을 메인 쓰레드로 넘길 경우
//lock free queue를 사용하면 됩니다.(UDP 쓰레드에서 메인 쓰레드로 패킷 넘기는 부분 참조)

BOOL ChatPacketEntry::ProcessPacket( BasePacket* pPacket)
{
	if(pPacket->GetPacketID() == CGSF::ChatRes)
	{
		SFProtobufPacket<ChatPacket::Chat>* pChat = (SFProtobufPacket<ChatPacket::Chat>*)pPacket;

		std::cout << pChat->GetData().chatmessage() << std::endl;

		return TRUE;
	}
	else if (pPacket->GetPacketType() == SFPACKET_DISCONNECT)
	{
//		g_pEngine->SetProcessing(FALSE);
		printf("Disconnect Server!!\n");
	}
	else if (pPacket->GetPacketType() == SFPACKET_CONNECT)
	{
		//g_pEngine->SetProcessing(TRUE);
		printf("Connect Server!!\n");

		m_Serial = pPacket->GetOwnerSerial();
	}

	return FALSE;

}