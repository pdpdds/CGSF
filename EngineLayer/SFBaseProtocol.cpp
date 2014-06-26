#include "stdafx.h"
#include "SFBaseProtocol.h"


SFBaseProtocol::SFBaseProtocol()
{
}


SFBaseProtocol::~SFBaseProtocol()
{
}

INT SFBaseProtocol::m_ioSize = PACKET_DEFAULT_IO_SIZE;

USHORT SFBaseProtocol::m_packetSize = PACKET_DEFAULT_PACKET_SIZE;

UINT SFBaseProtocol::m_packetOption = 0;

