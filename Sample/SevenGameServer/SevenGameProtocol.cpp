#include "stdafx.h"
#include "SevenGameProtocol.h"
#include "SFProtobufPacket.h"

SevenGameProtocol::SevenGameProtocol(void)
{
}


SevenGameProtocol::~SevenGameProtocol(void)
{
}

BasePacket* SevenGameProtocol::CreateIncomingPacketFromPacketId( int PacketId )
{
	BasePacket* pPacket = SFCGProtobufProtocol::CreateIncomingPacketFromPacketId(PacketId);

	if(pPacket != NULL)
		return pPacket;

	switch (PacketId)
	{
	case SevenGame::TurnPass:
		return new SFProtobufPacket<SevenGamePacket::TurnPass>(PacketId);
		break;
	case SevenGame::CardSubmit:
		return new SFProtobufPacket<SevenGamePacket::CardSubmit>(PacketId);
		break;

	default:
		SFASSERT(0);
		break;
	}
	return NULL;
}