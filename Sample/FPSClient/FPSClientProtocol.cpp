#include "stdafx.h"
#include "FPSClientProtocol.h"
#include "SFProtobufPacket.h"
#include "PacketID.h"
#include "PacketCore.pb.h"
#include "SFPacketStore.pb.h"

FPSClientProtocol::FPSClientProtocol(void)
{
}


FPSClientProtocol::~FPSClientProtocol(void)
{
}

BasePacket* FPSClientProtocol::CreateIncomingPacketFromPacketId( int PacketId )
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
	
	default:
		return NULL;
		break;
	}
	return NULL;
}
