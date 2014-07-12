#include "stdafx.h"
#include "SevenGameProtocol.h"
#include "SFProtobufPacket.h"

SevenGameProtocol::SevenGameProtocol(void)
{
}


SevenGameProtocol::~SevenGameProtocol(void)
{
}

BasePacket* SevenGameProtocol::CreateIncomingPacketFromPacketId( int packetId )
{
	BasePacket* pPacket = SFCGProtobufProtocol::CreateIncomingPacketFromPacketId(packetId);

	if(pPacket != NULL)
		return pPacket;

	switch (packetId)
	{
	case SevenGame::TurnPass:
		return new SFProtobufPacket<SevenGamePacket::TurnPass>(packetId);
	case SevenGame::CardSubmit:
		return new SFProtobufPacket<SevenGamePacket::CardSubmit>(packetId);
	default:
		SFASSERT(0);
		break;
	}

	return NULL;
}