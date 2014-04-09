#include "stdafx.h"
#include "SFJsonPacket.h"


SFJsonPacket::SFJsonPacket(USHORT usPacketId)
{
	m_Header.packetID = usPacketId;
}

SFJsonPacket::SFJsonPacket()
{
	
}


SFJsonPacket::~SFJsonPacket(void)
{
}
