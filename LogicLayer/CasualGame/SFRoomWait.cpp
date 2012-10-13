#include "StdAfx.h"
#include "SFRoomWait.h"
#include "SFRoom.h"
#include "SFPlayer.h"
#include "SFSendPacket.h"

SFRoomWait::SFRoomWait(SFRoom* pOwner, eRoomState State)
:SFRoomState(pOwner, State)
{
	m_Dispatch.RegisterMessage(CGSF::EnterRoom, std::tr1::bind(&SFRoomWait::OnEnterRoom, this, std::tr1::placeholders::_1));
	m_Dispatch.RegisterMessage(CGSF::LeaveRoom, std::tr1::bind(&SFRoomWait::OnLeaveRoom, this, std::tr1::placeholders::_1));

	m_DispatchingSystem.RegisterMessage(CGSF::ChangeTeam, std::tr1::bind(&SFRoomWait::OnChangeTeam, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2));
	m_DispatchingSystem.RegisterMessage(CGSF::StartGame, std::tr1::bind(&SFRoomWait::OnStartGame, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2));
	m_DispatchingSystem.RegisterMessage(CGSF::ChatReq, std::bind(&SFRoomWait::OnChat, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2));
}

SFRoomWait::~SFRoomWait(void)
{
}

BOOL SFRoomWait::OnEnterRoom( SFPlayer* pPlayer )
{
	SFRoom* pOwner = GetOwner();

	if(TRUE == pOwner->OnEnter(pPlayer))
	{
		pPlayer->ChangeState(PLAYER_STATE_ROOM);

		return TRUE;
	}

	return FALSE;
}

BOOL SFRoomWait::OnLeaveRoom( SFPlayer* pPlayer )
{
	SFRoom* pOwner = GetOwner();

	if(TRUE == pOwner->OnLeave(pPlayer))
	{
		pPlayer->ChangeState(PLAYER_STATE_LOBBY);

		SendLeaveRoom(pPlayer);

		return TRUE;
	}

	return FALSE;
}

BOOL SFRoomWait::ProcessUserRequest( SFPlayer* pPlayer, SFPacket* pPacket )
{
	if(FALSE == m_DispatchingSystem.HandleMessage(pPacket->GetPacketID(), pPlayer, pPacket))
		return m_Dispatch.HandleMessage(pPacket->GetPacketID(), pPlayer);
	
	return FALSE;
}

BOOL SFRoomWait::OnChangeTeam( SFPlayer* pPlayer, SFPacket* pPacket )
{
	SFRoom* pOwner = GetOwner();

	return pOwner->ChangeTeam(pPlayer);
}

BOOL SFRoomWait::OnStartGame( SFPlayer* pPlayer, SFPacket* pPacket )
{
	SFRoom* pOwner = GetOwner();

	if(pPlayer != pOwner->GetRoomChief())
	{
		return FALSE;
	}
	
	if(TRUE != pOwner->CheckPlayerState(PLAYER_STATE_ROOM))
		return FALSE;

	return pOwner->ChangeState(ROOM_STATE_LOADING);
}

BOOL SFRoomWait::OnChat( SFPlayer* pPlayer, SFPacket* pPacket )
{
	SFRoom* pRoom = GetOwner();

	SFPacketStore::ChatReq PktChatReq;
	protobuf::io::ArrayInputStream is(pPacket->GetDataBuffer(), pPacket->GetDataSize());
	PktChatReq.ParseFromZeroCopyStream(&is);

	SFPacketStore::ChatRes PktChatRes;
	PktChatRes.set_sender(pPlayer->m_username);
	PktChatRes.set_message(PktChatReq.message());

	int BufSize = PktChatRes.ByteSize();

	SFRoom::RoomMemberMap MemberMap = pRoom->GetRoomMemberMap();

	SFRoom::RoomMemberMap::iterator iter = MemberMap.begin();

	for(;iter != MemberMap.end(); iter++)
	{
		SFPlayer* pTarget = iter->second;

		if(pTarget == pPlayer)
			continue;

		SendToClient(pTarget, CGSF::ChatRes, &PktChatRes, BufSize);
	}

	return TRUE;
}