#include "stdafx.h"
#include "ProtocolServerProtocol.h"
#include "SFProtobufPacket.h"

ProtocolServerProtocol::ProtocolServerProtocol(void)
{
}


ProtocolServerProtocol::~ProtocolServerProtocol(void)
{
}

BasePacket* ProtocolServerProtocol::CreateIncomingPacketFromPacketId(int PacketId)
{	
	switch (PacketId)
	{
	case Protocol::Sample1:
		return new SFProtobufPacket<ProtocolPacket::Sample1>(PacketId);		
	case Protocol::Sample2:
		return new SFProtobufPacket<ProtocolPacket::Sample2>(PacketId);
	case Protocol::Sample3:
		return new SFProtobufPacket<ProtocolPacket::Sample3>(PacketId);
	case Protocol::Sample4:
		return new SFProtobufPacket<ProtocolPacket::Sample4>(PacketId);		

	default:
		SFASSERT(0);
		break;
	}
	return NULL;
}