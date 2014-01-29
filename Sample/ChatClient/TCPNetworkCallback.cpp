#include "StdAfx.h"
#include "TCPNetworkCallback.h"
#include "PacketID.h"
#include "SFEngine.h"
#include "BasePacket.h"
#include <iostream>

//extern SFEngine* g_pEngine;

using namespace google;

TCPNetworkCallback::TCPNetworkCallback(void)
{
}

TCPNetworkCallback::~TCPNetworkCallback(void)
{
}

bool TCPNetworkCallback::HandleNetworkMessage(BasePacket* pPacket)
{
	int PacketID = pPacket->GetPacketID();
	if(PacketID == CGSF::ChatReq)
	{
		/*ChatPacket::Chat PktChat;
		protobuf::io::ArrayInputStream is(pBuffer, Length);
		PktChat.ParseFromZeroCopyStream(&is);

		std::cout << PktChat.chatmessage() << std::endl;
		*/
		return TRUE;
	}

	return TRUE;
}