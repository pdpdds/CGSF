#include "stdafx.h"
#include "SevenGameProtocol.h"
#include "SFProtobufPacket.h"
#include <SFPacketStore/SFPacketID.h>
#include <SFPacketStore/SevenGamePacketID.h>
#include <SFPacketStore/PacketCore.pb.h>
#include <SFPacketStore/SFPacketStore.pb.h>
#include <SFPacketStore/SevenGamePacket.pb.h>

SevenGameProtocol::SevenGameProtocol(void)
{
}

SevenGameProtocol::~SevenGameProtocol(void)
{
}

BasePacket* SevenGameProtocol::CreateIncomingPacketFromPacketId( int packetId )
{
	BasePacket* pPacket = SFCGProtobufClientProtocol::CreateIncomingPacketFromPacketId(packetId);
	if(pPacket != NULL)
		return pPacket;

	switch (packetId)
	{
	case SevenGame::InitCardCount:
		return new SFProtobufPacket<SevenGamePacket::InitCardCount>(packetId);

	case SevenGame::MyCardInfo:
		return new SFProtobufPacket<SevenGamePacket::MyCardInfo>(packetId);
	case SevenGame::CurrentTurn:
		return new SFProtobufPacket<SevenGamePacket::CurrentTurn>(packetId);

	case SevenGame::TurnPass:
		return new SFProtobufPacket<SevenGamePacket::TurnPass>(packetId);

	case SevenGame::CardSubmit:
		return new SFProtobufPacket<SevenGamePacket::CardSubmit>(packetId);

	case SevenGame::Winner:
		return new SFProtobufPacket<SevenGamePacket::Winner>(packetId);

	case SevenGame::PlayerID:
		return new SFProtobufPacket<SevenGamePacket::PlayerID>(packetId);

	case SevenGame::TableUpdate:
		return new SFProtobufPacket<SevenGamePacket::TableUpdate>(packetId);

	case SevenGame::UserDie:
		return new SFProtobufPacket<SevenGamePacket::UserDie>(packetId);

	default:
		break;
	}
	return NULL;
}
