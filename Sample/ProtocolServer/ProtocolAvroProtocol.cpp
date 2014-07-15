#include "stdafx.h"
#include "ProtocolAvroProtocol.h"
#include "SFAvroPacket.h"
#include "SFPacketStore/AvroProtocolPacket.hh"


ProtocolAvroProtocol::ProtocolAvroProtocol()
{
}


ProtocolAvroProtocol::~ProtocolAvroProtocol()
{
}

BasePacket* ProtocolAvroProtocol::CreateIncomingPacketFromPacketId(int packetId)
{
	switch (packetId)
	{
	case Protocol::Sample1:
		return new SFAvroPacket<c::Sample1>(packetId);
	case Protocol::Sample2:
//		return new SFAvroPacket<ProtocolPacket::Sample2>(packetId);
	case Protocol::Sample3:
//		return new SFAvroPacket<ProtocolPacket::Sample3>(packetId);
	case Protocol::Sample4:
//		return new SFAvroPacket<ProtocolPacket::Sample4>(packetId);
	
	default:
		//SFASSERT(0);
		break;
	}
	return NULL;
}
