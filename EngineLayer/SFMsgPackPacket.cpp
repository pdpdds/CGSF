#include "stdafx.h"
#include "SFMsgPackPacket.h"

SFMsgPackPacket::SFMsgPackPacket(USHORT usPacketId)
{
	m_packetHeader.packetID = usPacketId;
	m_unpacker.reserve_buffer();
}

SFMsgPackPacket::SFMsgPackPacket()
{
	m_unpacker.reserve_buffer();
}


SFMsgPackPacket::~SFMsgPackPacket()
{
}

BasePacket* SFMsgPackPacket::Clone()
{
	return NULL;
}