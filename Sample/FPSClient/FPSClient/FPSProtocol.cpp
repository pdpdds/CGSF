#include "stdafx.h"
#include "FPSProtocol.h"
#include "SFProtobufPacket.h"
#include <SFPacketStore/FPSPacketID.h>
#include <SFPacketStore/FPSPacket.pb.h>

FPSProtocol::FPSProtocol(void)
{
}


FPSProtocol::~FPSProtocol(void)
{
}

BasePacket* FPSProtocol::CreateIncomingPacketFromPacketId(int PacketId)
{
	BasePacket* pPacket = SFCGProtobufClientProtocol::CreateIncomingPacketFromPacketId(PacketId);
	if (pPacket != NULL)
		return pPacket;

	switch (PacketId)
	{
	
	case FPS::MSG_SPAWN_PLAYER:
			return new SFProtobufPacket<FPSPacket::MSG_SPAWN_PLAYER>(PacketId);
		break;

	case  FPS::MSG_PLAYER_HEALTH:
		return new SFProtobufPacket<FPSPacket::MSG_PLAYER_HEALTH>(PacketId);
		break;

	case FPS::MSG_PLAYER_SCORE:
		return new SFProtobufPacket<FPSPacket::MSG_PLAYER_SCORE>(PacketId);
		break;

	case FPS::MSG_PLAYER_WEAPON_CHANGE:
		return new SFProtobufPacket<FPSPacket::MSG_PLAYER_WEAPON_CHANGE>(PacketId);
		break;

	case FPS::MSG_PLAYER_WEAPON_CHANGING:
		return new SFProtobufPacket<FPSPacket::MSG_PLAYER_WEAPON_CHANGING>(PacketId);
		break;
	
	default:		
		return NULL;
		break;
	}
	return NULL;
}