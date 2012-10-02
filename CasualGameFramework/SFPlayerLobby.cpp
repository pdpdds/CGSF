#include "StdAfx.h"
#include "SFPlayerLobby.h"
#include "SFPlayer.h"
#include "SFPacketStore.pb.h"
#include "PacketID.h"
#include "DBMsg.h"
#include "SFRoomManager.h"
#include "SFSendPacket.h"
#include "SFLobby.h"
#include "GamePacketStructure.h"

SFPlayerLobby::SFPlayerLobby(SFPlayer* pOwner, ePlayerState State)
: SFPlayerState(pOwner, State)
{
}

SFPlayerLobby::~SFPlayerLobby(void)
{
}

BOOL SFPlayerLobby::OnEnter()
{
	SFRoomManager* pRoomManager = SFLogicEntry::GetLogicEntry()->GetRoomManager();
	pRoomManager->SendRoomPage(this);

	SendEnterLobby(GetOwner());

	SFLobby* pLobby =SFLogicEntry::GetLogicEntry()->GetLobby();

	pLobby->OnEnter(GetOwner());


	return TRUE;
}

BOOL SFPlayerLobby::OnLeave()
{
	SFLobby* pLobby = SFLogicEntry::GetLogicEntry()->GetLobby();

	pLobby->OnLeave(GetOwner());

	return TRUE;
}

BOOL SFPlayerLobby::ProcessPacket( SFPacket* pPacket )
{
	switch(pPacket->GetPacketID())
	{
	case CGSF::EnterRoom:
		{
			OnEnterRoom(pPacket);
		}
		break;
	case CGSF::CreateRoom:
		{
			OnCreateRoom(pPacket);
		}
		break;
	case CGSF::ChatReq:
		{
			OnChatReq(pPacket);
		}
		break;

	case CGSF::PlayerIP:
		{
			OnPlayerIP(pPacket);
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

BOOL SFPlayerLobby::ProcessDBResult( SFMessage* pMessage )
{
	return TRUE;
}

BOOL SFPlayerLobby::OnEnterRoom( SFPacket* pPacket )
{
	SFPlayer* pPlayer = GetOwner();
	
	_PeerInfo Info = pPlayer->GetIPInfo();
	if(Info.ExternalIP == 0)
		return FALSE;

	SFRoomManager* pRoomManager = SFLogicEntry::GetLogicEntry()->GetRoomManager();

	SFPacketStore::EnterRoom PkEnterRoom;
	protobuf::io::ArrayInputStream is(pPacket->GetDataBuffer(), pPacket->GetDataSize());
	PkEnterRoom.ParseFromZeroCopyStream(&is);

	int RoomIndex = PkEnterRoom.roomindex();

	SFRoom* pRoom = pRoomManager->GetRoom(RoomIndex);

	if(pRoom == NULL)
		return FALSE;

	if(TRUE == pRoom->ProcessUserRequest(pPlayer, pPacket))
		SendToClient(GetOwner(), CGSF::EnterRoom, &PkEnterRoom, PkEnterRoom.ByteSize());

	return TRUE;
}

BOOL SFPlayerLobby::OnCreateRoom( SFPacket* pPacket )
{
	SFPlayer* pPlayer = GetOwner();
	_PeerInfo Info = pPlayer->GetIPInfo();
	if(Info.ExternalIP == 0)
		return FALSE;

	SFRoomManager* pRoomManager = SFLogicEntry::GetLogicEntry()->GetRoomManager();

	SFPacketStore::CreateRoom PktCreateRoom;
	protobuf::io::ArrayInputStream is(pPacket->GetDataBuffer(), pPacket->GetDataSize());
	PktCreateRoom.ParseFromZeroCopyStream(&is);

	BOOL bResult = pRoomManager->OnCreateRoom(this, PktCreateRoom.gamemode());

	if(FALSE == bResult)
		return FALSE;

	pPlayer->ChangeState(PLAYER_STATE_ROOM);

	SendToClient(GetOwner(), CGSF::CreateRoom, &PktCreateRoom, PktCreateRoom.ByteSize());

	return TRUE;
}

BOOL SFPlayerLobby::OnChatReq( SFPacket* pPacket )
{
	SFLobby* pLobby = SFLogicEntry::GetLogicEntry()->GetLobby();

	SFPlayer* pPlayer = GetOwner();

	SFPacketStore::ChatReq PktChatReq;
	protobuf::io::ArrayInputStream is(pPacket->GetDataBuffer(), pPacket->GetDataSize());
	PktChatReq.ParseFromZeroCopyStream(&is);

	pLobby->OnChat(GetOwner(), PktChatReq.message());

	return TRUE;
}

BOOL SFPlayerLobby::OnPlayerIP( SFPacket* pPacket )
{
	SFPlayer* pPlayer = GetOwner();

	SFPacketStore::PLAYER_IP PktPlayerIP;
	protobuf::io::ArrayInputStream is(pPacket->GetDataBuffer(), pPacket->GetDataSize());
	PktPlayerIP.ParseFromZeroCopyStream(&is);

	PlayerIPMsg Msg;
	SF_GETPACKET_ARG(&Msg, PktPlayerIP.playerip(), PlayerIPMsg);

	_PeerInfo Info;
	Info.ExternalIP = Msg.ExternalIP;
	Info.ExternalPort = Msg.ExternalPort;
	Info.LocalIP = Msg.LocalIP;
	Info.LocalPort = Msg.LocalPort;

	pPlayer->SetIPInfo(Info);

	return TRUE;
}