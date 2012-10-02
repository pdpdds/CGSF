#include "StdAfx.h"
#include "TCPNetworkCallback.h"
#include "PacketID.h"
#include "SFEngine.h"
#include "SFPacket.h"
#include <iostream>
#include "SFClient.h"
#include "GoogleLog.h"

extern SFSYSTEM_CLIENT* g_pNetworkEngine;

using namespace google;

TCPNetworkCallback::TCPNetworkCallback(void)
{
}

TCPNetworkCallback::~TCPNetworkCallback(void)
{
}

bool TCPNetworkCallback::HandleNetworkMessage(int PacketID, BYTE* pBuffer, USHORT Length)
{
	if(PacketID == CGSF::ChatReq)
	{
		ChatPacket::Chat PktChat;
		protobuf::io::ArrayInputStream is(pBuffer, Length);
		PktChat.ParseFromZeroCopyStream(&is);

		std::cout << PktChat.chatmessage() << std::endl;

		return TRUE;
	}

	return TRUE;
}