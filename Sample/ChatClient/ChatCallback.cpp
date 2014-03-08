#include "StdAfx.h"
#include "ChatCallback.h"
#include "BasePacket.h"
#include <iostream>
#include "SFJsonPacket.h"

ChatCallback::ChatCallback(void)
{
}

ChatCallback::~ChatCallback(void)
{
}

bool ChatCallback::HandleNetworkMessage(BasePacket* pPacket)
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;
	int PacketID = pJsonPacket->GetData().GetValue<int>("PacketId");

	if (PacketID == CHAT_PACKET_NUM)
	{
		std::cout << "Received : " << pJsonPacket->GetData().GetValue<tstring>("chat") << std::endl;

		return TRUE;
	}

	return FALSE;
}