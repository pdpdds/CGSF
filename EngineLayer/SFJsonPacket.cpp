#include "stdafx.h"
#include "SFJsonPacket.h"


SFJsonPacket::SFJsonPacket(USHORT usPacketId)
{
	m_Node.Add("PacketId", usPacketId);
}

SFJsonPacket::SFJsonPacket()
{
	
}


SFJsonPacket::~SFJsonPacket(void)
{
}
