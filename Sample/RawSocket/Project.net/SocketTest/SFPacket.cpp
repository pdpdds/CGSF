#include "StdAfx.h"
#include "SFPacket.h"

SFPacket::SFPacket(unsigned short packetID)
{
	m_Header.packetID = packetID;

	Initialize();
}

SFPacket::SFPacket()
{
	m_Header.packetID = 0;

	Initialize();
}

SFPacket::~SFPacket(void)
{
}

bool SFPacket::Initialize()
{
	m_Header.packetOption = 0;
	m_Header.dataCRC = 0;
	m_Header.dataSize = 0;

	ResetDataBuffer();

	m_usCurrentReadPosition = 0;

	return true;
}

void SFPacket::ResetDataBuffer()
{
	memset(m_pPacketData, 0, sizeof(MAX_PACKET_DATA));
}