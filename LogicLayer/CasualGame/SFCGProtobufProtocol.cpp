#include "stdafx.h"
#include "SFCGProtobufProtocol.h"
#include "SFProtobufPacket.h"

SFCGProtobufProtocol::SFCGProtobufProtocol(void)
{
}


SFCGProtobufProtocol::~SFCGProtobufProtocol(void)
{
}

BasePacket* SFCGProtobufProtocol::CreateIncomingPacketFromPacketId( int PacketId )
{
	switch (PacketId)
	{
	case CGSF::EnterLobby:
		return new SFProtobufPacket<SFPacketStore::EnterLobby>(PacketId);
		break;

	case CGSF::Login:
		return new SFProtobufPacket<SFPacketStore::Login>(PacketId);
		break;

	case CGSF::CreateRoom:
		return new SFProtobufPacket<SFPacketStore::CreateRoom>(PacketId);
		break;

	default:
		return NULL;
		break;
	}
}

