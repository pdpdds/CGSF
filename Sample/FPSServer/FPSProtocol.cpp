#include "stdafx.h"
#include "FPSProtocol.h"
#include "SFProtobufPacket.h"

FPSProtocol::FPSProtocol(void)
{
}


FPSProtocol::~FPSProtocol(void)
{
}

BasePacket* FPSProtocol::CreateIncomingPacketFromPacketId( int PacketId )
{
	BasePacket* pPacket = SFCGProtobufProtocol::CreateIncomingPacketFromPacketId(PacketId);

	if(pPacket != NULL)
		return pPacket;

	switch (PacketId)
	{
	case CGSF::MSG_PLAYER_HEALTH:
		return new SFProtobufPacket<SFPacketStore::MSG_PLAYER_HEALTH>(PacketId);
		break;
	case CGSF::MSG_SPAWN_PLAYER:
		return new SFProtobufPacket<SFPacketStore::MSG_SPAWN_PLAYER>(PacketId);
		break;
	case CGSF::MSG_PLAYER_SCORE:
		return new SFProtobufPacket<SFPacketStore::MSG_PLAYER_SCORE>(PacketId);
		break;
	case CGSF::MSG_PLAYER_WEAPON_CHANGE:
		return new SFProtobufPacket<SFPacketStore::MSG_PLAYER_WEAPON_CHANGE>(PacketId);
		break;
	case CGSF::MSG_PLAYER_WEAPON_CHANGING:
		return new SFProtobufPacket<SFPacketStore::MSG_PLAYER_WEAPON_CHANGING>(PacketId);
		break;

	default:
		return NULL;
		break;
	}
	return NULL;
}