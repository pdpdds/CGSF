#include "stdafx.h"
#include "ProtocolServerProtocol.h"
#include "SFProtobufPacket.h"

ProtocolServerProtocol::ProtocolServerProtocol(void)
{
}


ProtocolServerProtocol::~ProtocolServerProtocol(void)
{
}

BasePacket* ProtocolServerProtocol::CreateIncomingPacketFromPacketId(int packetId)
{	
	switch (packetId)
	{
	case Protocol::Sample1:
		return new SFProtobufPacket<ProtocolPacket::Sample1>(packetId);

	case Protocol::Sample2:
		return new SFProtobufPacket<ProtocolPacket::Sample2>(packetId);

	case Protocol::Sample3:
		return new SFProtobufPacket<ProtocolPacket::Sample3>(packetId);

	case Protocol::Sample4:
		return new SFProtobufPacket<ProtocolPacket::Sample4>(packetId);

	default:
		SFASSERT(0);
		break;
	}
	return NULL;
}