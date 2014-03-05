#include "stdafx.h"
#include "CasualGameProtocol.h"
#include "SFProtobufPacket.h"
#include <SFPacketStore/PacketID.h>
#include <SFPacketStore/SFPacketStore.pb.h>
#include <SFPacketStore/SevenGamePacketID.h>
#include <SFPacketStore/PacketCore.pb.h>
#include <SFPacketStore/SevenGamePacket.pb.h>

CasualGameProtocol::CasualGameProtocol(void)
{
}


CasualGameProtocol::~CasualGameProtocol(void)
{
}

BasePacket* CasualGameProtocol::CreateIncomingPacketFromPacketId( int PacketId )
{
	switch (PacketId)
	{
	case CGSF::Auth:
		return new SFProtobufPacket<PacketCore::Auth>(PacketId);
		break;

	case CGSF::LoginSuccess:
		return new SFProtobufPacket<SFPacketStore::LoginSuccess>(PacketId);
		break;

	case CGSF::LoginFail:
		return new SFProtobufPacket<SFPacketStore::LoginFail>(PacketId);
		break;

	case CGSF::EnterLobby:
		return new SFProtobufPacket<SFPacketStore::EnterLobby>(PacketId);
		break;

	case CGSF::CreateRoom:
		return new SFProtobufPacket<SFPacketStore::CreateRoom>(PacketId);
		break;

	case CGSF::EnterRoom:
		return new SFProtobufPacket<SFPacketStore::EnterRoom>(PacketId);
		break;

	case CGSF::LeaveRoom:
		return new SFProtobufPacket<SFPacketStore::LeaveRoom>(PacketId);
		break;

	case CGSF::EnterTeamMember:
		return new SFProtobufPacket<SFPacketStore::EnterTeamMember>(PacketId);
		break;

	case CGSF::LeaveTeamMember:
		return new SFProtobufPacket<SFPacketStore::LeaveTeamMember>(PacketId);
		break;

	case CGSF::RoomList:
		return new SFProtobufPacket<SFPacketStore::RoomList>(PacketId);
		break;

	case CGSF::RoomMember:
		return new SFProtobufPacket<SFPacketStore::RoomMember>(PacketId);
		break;

	case CGSF::ChatRes:
		return new SFProtobufPacket<SFPacketStore::ChatRes>(PacketId);
		break;

	case CGSF::LoadingStart:
		return new SFProtobufPacket<SFPacketStore::LoadingStart>(PacketId);
		break;

	case CGSF::PlayStart:
		return new SFProtobufPacket<SFPacketStore::PlayStart>(PacketId);
		break;

	case CGSF::PlayEnd:
		return new SFProtobufPacket<SFPacketStore::PlayStart>(PacketId);
		break;

	case CGSF::PeerList:
		return new SFProtobufPacket<SFPacketStore::PeerList>(PacketId);
		break;

	case CGSF::MSG_CREATE_PLAYER:
		return new SFProtobufPacket<SFPacketStore::MSG_CREATE_PLAYER>(PacketId);
		break;

	case CGSF::MSG_DESTROY_PLAYER:
		return new SFProtobufPacket<SFPacketStore::MSG_DESTROY_PLAYER>(PacketId);
		break;

	case CGSF::AddPeer:
		return new SFProtobufPacket<SFPacketStore::ADD_PEER>(PacketId);
		break;

	case CGSF::DeletePeer:
		return new SFProtobufPacket<SFPacketStore::DELETE_PEER>(PacketId);
		break;

	default:
		return NULL;
		break;
	}
	return NULL;
}
