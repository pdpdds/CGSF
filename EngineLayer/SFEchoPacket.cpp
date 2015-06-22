#include "stdafx.h"
#include "SFEchoPacket.h"

#define nReceiveBufferSize 1024 * 8/*8192*/
#define nSendBufferSize 1024 * 8/*8192*/
SFEchoPacket::SFEchoPacket()
	: m_Buffer(nReceiveBufferSize)
{
}


SFEchoPacket::~SFEchoPacket()
{
}
