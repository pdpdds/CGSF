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

	if (pJsonPacket->GetPacketID() == CHAT_PACKET_NUM)
	{
		std::cout << pJsonPacket->GetData().GetValue<tstring>("who") << " : " << pJsonPacket->GetData().GetValue<tstring>("chat") << std::endl;

		return TRUE;
	}

	return FALSE;
}