#include "stdafx.h"
#include "SevenGameProtocol.h"
#include "SFProtobufPacket.h"
#include "PacketID.h"
#include "SevenGamePacketID.h"
#include "PacketCore.pb.h"
#include "SFPacketStore.pb.h"
#include "SevenGamePacket.pb.h"

SevenGameProtocol::SevenGameProtocol(void)
{
}

SevenGameProtocol::~SevenGameProtocol(void)
{
}

BasePacket* SevenGameProtocol::CreateIncomingPacketFromPacketId( int PacketId )
{
	BasePacket* pPacket = CasualGameProtocol::CreateIncomingPacketFromPacketId(PacketId);
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
	case SevenGame::PlayerID:
		return new SFProtobufPacket<SevenGamePacket::PlayerID>(PacketId);
		break;
	case SevenGame::TableUpdate:
		return new SFProtobufPacket<SevenGamePacket::TableUpdate>(PacketId);
		break;
	case SevenGame::UserDie:
		return new SFProtobufPacket<SevenGamePacket::UserDie>(PacketId);
		break;

	default:
		return NULL;
		break;
	}
	return NULL;
}
