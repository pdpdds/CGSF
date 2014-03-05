#include "stdafx.h"
#include "ChatClientProtocol.h"
#include "SFProtobufPacket.h"
#include <SFPacketStore/PacketID.h>
#include <SFPacketStore/ChatPacket.pb.h>
#include <SFPacketStore/PacketCore.pb.h>

ChatClientProtocol::ChatClientProtocol(void)
{
}


ChatClientProtocol::~ChatClientProtocol(void)
{
}

BasePacket* ChatClientProtocol::CreateIncomingPacketFromPacketId( int PacketId )
{
	switch (PacketId)
	{
	case CGSF::Auth:
		return new SFProtobufPacket<PacketCore::Auth>(PacketId);
		break;

	case CGSF::ChatRes:
		return new SFProtobufPacket<ChatPacket::Chat>(PacketId);
		break;

	default:
		return NULL;
		break;
	}
	return NULL;
}
