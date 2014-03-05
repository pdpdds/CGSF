#include "stdafx.h"
#include "ChatProtocol.h"
#include "SFProtobufPacket.h"
#include <SFPacketStore/PacketID.h>
#include <SFPacketStore/ChatPacket.pb.h>

ChatProtocol::ChatProtocol(void)
{
}


ChatProtocol::~ChatProtocol(void)
{
}

BasePacket* ChatProtocol::CreateIncomingPacketFromPacketId( int PacketId )
{
	switch (PacketId)
	{
	case CGSF::ChatReq:
		return new SFProtobufPacket<ChatPacket::Chat>(PacketId);
		break;

	default:
		return NULL;
		break;
	}
	return NULL;
}
