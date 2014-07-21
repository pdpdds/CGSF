#include "stdafx.h"
#include "SFJsonPacket.h"
#include "SFJsonProtocol.h"

SFJsonPacket::SFJsonPacket(USHORT usPacketId)
{
	m_packetHeader.packetID = usPacketId;
}

SFJsonPacket::SFJsonPacket()
{
	
}

SFJsonPacket::~SFJsonPacket(void)
{
}

BasePacket* SFJsonPacket::Clone()
{
	SFJsonPacket* pClone = new SFJsonPacket();
	pClone->CopyBaseHeader(this);
	pClone->m_packetHeader = m_packetHeader;
	pClone->m_Node = m_Node;

	return pClone;
}

void SFJsonPacket::Release()
{
	SFJsonProtocol::DisposePacket(this);
}