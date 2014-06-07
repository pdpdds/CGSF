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

BasePacket* ProtocolAvroProtocol::CreateIncomingPacketFromPacketId(int PacketId)
{
	switch (PacketId)
	{
	case Protocol::Sample1:
		return new SFAvroPacket<c::Sample1>(PacketId);
	case Protocol::Sample2:
//		return new SFAvroPacket<ProtocolPacket::Sample2>(PacketId);
	case Protocol::Sample3:
//		return new SFAvroPacket<ProtocolPacket::Sample3>(PacketId);
	case Protocol::Sample4:
//		return new SFAvroPacket<ProtocolPacket::Sample4>(PacketId);
	
	default:
		//SFASSERT(0);
		break;
	}
	return NULL;
}
