#include "stdafx.h"
#include "SFCGProtobufClientProtocol.h"
#include "SFProtobufPacket.h"
#include <SFPacketStore/SFPacketID.h>
#include <SFPacketStore/SFPacketStore.pb.h>
#include <SFPacketStore/SevenGamePacketID.h>
#include <SFPacketStore/PacketCore.pb.h>

SFCGProtobufClientProtocol::SFCGProtobufClientProtocol(void)
{
}


SFCGProtobufClientProtocol::~SFCGProtobufClientProtocol(void)
{
}

BasePacket* SFCGProtobufClientProtocol::CreateIncomingPacketFromPacketId(int packetId)
{
	switch (packetId)
	{
	case CGSF::Auth:
		return new SFProtobufPacket<PacketCore::Auth>(packetId);

	case CGSF::LoginSuccess:
		return new SFProtobufPacket<SFPacketStore::LoginSuccess>(packetId);

	case CGSF::LoginFail:
		return new SFProtobufPacket<SFPacketStore::LoginFail>(packetId);

	case CGSF::EnterLobby:
		return new SFProtobufPacket<SFPacketStore::EnterLobby>(packetId);

	case CGSF::CreateRoom:
		return new SFProtobufPacket<SFPacketStore::CreateRoom>(packetId);
		break;

	case CGSF::EnterRoom:
		return new SFProtobufPacket<SFPacketStore::EnterRoom>(packetId);

	case CGSF::LeaveRoom:
		return new SFProtobufPacket<SFPacketStore::LeaveRoom>(packetId);

	case CGSF::EnterTeamMember:
		return new SFProtobufPacket<SFPacketStore::EnterTeamMember>(packetId);

	case CGSF::LeaveTeamMember:
		return new SFProtobufPacket<SFPacketStore::LeaveTeamMember>(packetId);

	case CGSF::RoomList:
		return new SFProtobufPacket<SFPacketStore::RoomList>(packetId);

	case CGSF::RoomMember:
		return new SFProtobufPacket<SFPacketStore::RoomMember>(packetId);

	case CGSF::ChatRes:
		return new SFProtobufPacket<SFPacketStore::ChatRes>(packetId);

	case CGSF::LoadingStart:
		return new SFProtobufPacket<SFPacketStore::LoadingStart>(packetId);

	case CGSF::PlayStart:
		return new SFProtobufPacket<SFPacketStore::PlayStart>(packetId);

	case CGSF::PlayEnd:
		return new SFProtobufPacket<SFPacketStore::PlayStart>(packetId);

///////////////////////////////////////////////////////////////////////
//P2P와 관계된 기능. P2P를 이용하지 않는다면 신경쓰지 않아도 된다.
//////////////////////////////////////////////////////////////////////
	case CGSF::PeerList:
		return new SFProtobufPacket<SFPacketStore::PeerList>(packetId);

	case CGSF::AddPeer:
		return new SFProtobufPacket<SFPacketStore::ADD_PEER>(packetId);

	case CGSF::DeletePeer:
		return new SFProtobufPacket<SFPacketStore::DELETE_PEER>(packetId);

	case CGSF::MSG_CREATE_PLAYER:
		return new SFProtobufPacket<SFPacketStore::MSG_CREATE_PLAYER>(packetId);

	case CGSF::MSG_DESTROY_PLAYER:
		return new SFProtobufPacket<SFPacketStore::MSG_DESTROY_PLAYER>(packetId);
	}

	return NULL;
}