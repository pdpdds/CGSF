#include "stdafx.h"
#include "SFGameProtocol.h"
#include "SFProtobufPacket.h"
#include <SFPacketStore/PacketCore.pb.h>
#include <SFPacketStore/PacketID.h>
#include <SFPacketStore/SFPacketStore.pb.h>

SFGameProtocol::SFGameProtocol(void)
{
}


SFGameProtocol::~SFGameProtocol(void)
{
}

BasePacket* SFGameProtocol::CreateIncomingPacketFromPacketId( int PacketId )
{
	BasePacket* pPacket = CasualGameProtocol::CreateIncomingPacketFromPacketId(PacketId);
	if (pPacket != NULL)
		return pPacket;

	switch (PacketId)
	{

	case CGSF::PeerList:
			return new SFProtobufPacket<SFPacketStore::PeerList>(PacketId);
		break;
	case CGSF::AddPeer:
		return new SFProtobufPacket<SFPacketStore::ADD_PEER>(PacketId);
		break;
	case CGSF::PlayStart:
		return new SFProtobufPacket<SFPacketStore::PlayStart>(PacketId);
		break;
	case CGSF::PlayEnd:
		return new SFProtobufPacket<SFPacketStore::PlayEnd>(PacketId);
		break;


	case CGSF::DeletePeer:
		return new SFProtobufPacket<SFPacketStore::DELETE_PEER>(PacketId);
		break;

	case CGSF::MSG_CREATE_PLAYER:
			return new SFProtobufPacket<SFPacketStore::MSG_CREATE_PLAYER>(PacketId);
		break;

	case CGSF::MSG_DESTROY_PLAYER:
			return new SFProtobufPacket<SFPacketStore::MSG_DESTROY_PLAYER>(PacketId);
		break;

	case CGSF::MSG_SPAWN_PLAYER:
			return new SFProtobufPacket<SFPacketStore::MSG_SPAWN_PLAYER>(PacketId);
		break;

	case  CGSF::MSG_PLAYER_HEALTH:
			return new SFProtobufPacket<SFPacketStore::MSG_PLAYER_HEALTH>(PacketId);
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