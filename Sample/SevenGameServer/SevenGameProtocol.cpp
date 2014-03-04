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
	case SevenGame::InitCardCount:
		return new SFProtobufPacket<SevenGamePacket::InitCardCount>(PacketId);
		break;
	case SevenGame::MyCardInfo:
		return new SFProtobufPacket<SevenGamePacket::MyCardInfo>(PacketId);
		break;
	case SevenGame::CurrentTurn:
		return new SFProtobufPacket<SevenGamePacket::CurrentTurn>(PacketId);
		break;
	case SevenGame::TurnPass:
		return new SFProtobufPacket<SevenGamePacket::TurnPass>(PacketId);
		break;
	case SevenGame::CardSubmit:
		return new SFProtobufPacket<SevenGamePacket::CardSubmit>(PacketId);
		break;
	case SevenGame::Winner:
		return new SFProtobufPacket<SevenGamePacket::Winner>(PacketId);
		break;

	default:
		return NULL;
		break;
	}
	return NULL;
}