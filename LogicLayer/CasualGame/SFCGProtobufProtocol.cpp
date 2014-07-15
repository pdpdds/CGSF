#include "stdafx.h"
#include "SFCGProtobufProtocol.h"
#include "SFProtobufPacket.h"

SFCGProtobufProtocol::SFCGProtobufProtocol(void)
{
}


SFCGProtobufProtocol::~SFCGProtobufProtocol(void)
{
}

BasePacket* SFCGProtobufProtocol::CreateIncomingPacketFromPacketId(int packetId)
{
	switch (packetId)
	{
	case CGSF::EnterLobby:
		return new SFProtobufPacket<SFPacketStore::EnterLobby>(packetId);		

	case CGSF::Login:
		return new SFProtobufPacket<SFPacketStore::Login>(packetId);

	case CGSF::CreateRoom:
		return new SFProtobufPacket<SFPacketStore::CreateRoom>(packetId);

	case CGSF::EnterRoom:
		return new SFProtobufPacket<SFPacketStore::EnterRoom>(packetId);

	case CGSF::PlayerIP:
		return new SFProtobufPacket<SFPacketStore::PLAYER_IP>(packetId);

	case CGSF::ChatReq:
		return new SFProtobufPacket<SFPacketStore::ChatReq>(packetId);

	case CGSF::LeaveRoom:
		return new SFProtobufPacket<SFPacketStore::LeaveRoom>(packetId);
	
	case CGSF::ChangeTeam:
		return new SFProtobufPacket<SFPacketStore::ChangeTeam>(packetId);

	case CGSF::LoadingStart:
		return new SFProtobufPacket<SFPacketStore::LoadingStart>(packetId);
	
	case CGSF::LoadingComplete:
		return new SFProtobufPacket<SFPacketStore::LoadingComplete>(packetId);

	case CGSF::RoomListRefresh:
		return new SFProtobufPacket<SFPacketStore::RoomListRefresh>(packetId);
	}

	return NULL;
}

