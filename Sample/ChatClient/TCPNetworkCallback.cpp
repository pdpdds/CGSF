#include "StdAfx.h"
#include "TCPNetworkCallback.h"
#include <SFPacketStore/PacketID.h>
#include "SFEngine.h"
#include "BasePacket.h"
#include <iostream>
#include "SFJsonPacket.h"

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
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;
	int PacketID = pJsonPacket->GetData().GetValue<int>("PacketId");

	if(PacketID == 1234)
	{
		std::cout << pJsonPacket->GetData().GetValue<tstring>("chat") << std::endl;

		return TRUE;
	}

	return FALSE;
}