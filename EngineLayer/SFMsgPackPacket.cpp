#include "stdafx.h"
#include "SFMsgPackPacket.h"
#include "SFMsgPackProtocol.h"

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

void SFMsgPackPacket::Release()
{
	SFMsgPackProtocol::DisposePacket(this);
}