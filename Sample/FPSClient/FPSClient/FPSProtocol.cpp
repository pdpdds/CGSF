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

BasePacket* FPSProtocol::CreateIncomingPacketFromPacketId(int packetId)
{
	BasePacket* pPacket = SFCGProtobufClientProtocol::CreateIncomingPacketFromPacketId(packetId);
	if (pPacket != NULL)
		return pPacket;

	switch (packetId)
	{
	
	case FPS::MSG_SPAWN_PLAYER:
		return new SFProtobufPacket<FPSPacket::MSG_SPAWN_PLAYER>(packetId);

	case  FPS::MSG_PLAYER_HEALTH:
		return new SFProtobufPacket<FPSPacket::MSG_PLAYER_HEALTH>(packetId);

	case FPS::MSG_PLAYER_SCORE:
		return new SFProtobufPacket<FPSPacket::MSG_PLAYER_SCORE>(packetId);

	case FPS::MSG_PLAYER_WEAPON_CHANGE:
		return new SFProtobufPacket<FPSPacket::MSG_PLAYER_WEAPON_CHANGE>(packetId);

	case FPS::MSG_PLAYER_WEAPON_CHANGING:
		return new SFProtobufPacket<FPSPacket::MSG_PLAYER_WEAPON_CHANGING>(packetId);
	}
	return NULL;
}