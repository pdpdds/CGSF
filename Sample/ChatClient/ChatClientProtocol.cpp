#include "stdafx.h"
#include "ChatClientProtocol.h"
#include "SFProtobufPacket.h"
#include "PacketID.h"
#include "ChatPacket.pb.h"
#include "PacketCore.pb.h"

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
