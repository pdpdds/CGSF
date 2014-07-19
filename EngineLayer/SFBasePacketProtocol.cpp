#include "stdafx.h"
#include "SFBasePacketProtocol.h"


SFBasePacketProtocol::SFBasePacketProtocol()
{
}


SFBasePacketProtocol::~SFBasePacketProtocol()
{
}

void SFBasePacketProtocol::CopyBaseProtocol(SFBasePacketProtocol& source)
{
	m_ioSize = source.GetIOSize();
	m_packetSize = source.GetPacketSize();
	m_packetOption = source.GetPacketOption();
}
