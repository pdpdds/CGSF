#include "stdafx.h"
#include "SFCGProtobufProtocol.h"
#include "SFProtobufPacket.h"

SFCGProtobufProtocol::SFCGProtobufProtocol(void)
{
}


SFCGProtobufProtocol::~SFCGProtobufProtocol(void)
{
}

BasePacket* SFCGProtobufProtocol::CreateIncomingPacketFromPacketId( int PacketId )
{
	switch (PacketId)
	{
	case CGSF::EnterLobby:
		return new SFProtobufPacket<SFPacketStore::EnterLobby>(PacketId);
		break;

	case CGSF::Login:
		return new SFProtobufPacket<SFPacketStore::Login>(PacketId);
		break;

	case CGSF::CreateRoom:
		return new SFProtobufPacket<SFPacketStore::CreateRoom>(PacketId);
		break;

	case CGSF::EnterRoom:
		return new SFProtobufPacket<SFPacketStore::EnterRoom>(PacketId);
		break;

	case CGSF::PlayerIP:
		return new SFProtobufPacket<SFPacketStore::PLAYER_IP>(PacketId);
		break;

	case CGSF::ChatReq:
		return new SFProtobufPacket<SFPacketStore::ChatReq>(PacketId);
		break;


	case CGSF::LeaveRoom:
		return new SFProtobufPacket<SFPacketStore::LeaveRoom>(PacketId);
		break;

		
	case CGSF::ChangeTeam:
		return new SFProtobufPacket<SFPacketStore::ChangeTeam>(PacketId);
		break;

	case CGSF::LoadingStart:
		return new SFProtobufPacket<SFPacketStore::LoadingStart>(PacketId);
		break;
	
	case CGSF::LoadingComplete:
		return new SFProtobufPacket<SFPacketStore::LoadingComplete>(PacketId);
		break;

	case CGSF::RoomListRefresh:
		return new SFProtobufPacket<SFPacketStore::RoomListRefresh>(PacketId);
		break;


	default:
		return NULL;
		break;
	}
}

