#include "StdAfx.h"
#include "SFRoomLoading.h"
#include "SFRoom.h"
#include "SFPlayer.h"

SFRoomLoading::SFRoomLoading(SFRoom* pOwner, eRoomState State)
: SFRoomState(pOwner, State)
{
	m_DispatchingSystem.RegisterMessage(CGSF::LoadingComplete, std::tr1::bind(&SFRoomLoading::OnLoadingComplete, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2));
}

SFRoomLoading::~SFRoomLoading(void)
{
}

BOOL SFRoomLoading::OnEnter()
{
	SFRoom* pRoom = GetOwner();
	pRoom->ChangePlayerFSM(PLAYER_STATE_LOADING);

	return TRUE;
}

BOOL SFRoomLoading::OnLeave()
{
	return TRUE;
}

BOOL SFRoomLoading::ProcessUserRequest( SFPlayer* pPlayer, BasePacket* pPacket )
{
	return m_DispatchingSystem.HandleMessage(pPacket->GetPacketID(), pPlayer, pPacket);
}

BOOL SFRoomLoading::OnLoadingComplete( SFPlayer* pPlayer, BasePacket* pPacket)
{
	SFRoom* pRoom = GetOwner();

	pRoom->GetP2PSys()->AddPeer(pPlayer->GetSerial(), pPlayer->GetIPInfo());

	if(TRUE != pRoom->CheckLoadingComplete())
		return FALSE;

	pRoom->ChangeState(ROOM_STATE_PLAYREADY);
	pRoom->ChangeState(ROOM_STATE_PLAY);

	return TRUE;
}

BOOL SFRoomLoading::OnLeaveRoom( SFPlayer* pPlayer )
{
	SFRoom* pOwner = GetOwner();

	if(TRUE == pOwner->OnLeave(pPlayer))
	{
		SFMessage Message;
		Message.Initialize(CGSF::MSG_DESTROY_PLAYER);
		Message << pPlayer->GetSerial();

		pOwner->PropagateMessage(&Message);

		Message.Initialize(CGSF::DeletePeer);
		Message << pPlayer->GetSerial();

		pOwner->PropagateMessage(&Message);

		pOwner->GetP2PSys()->DeletePeer(pPlayer->GetSerial());


		pPlayer->ChangeState(PLAYER_STATE_LOBBY);

		SendLeaveRoom(pPlayer);

		return TRUE;
	}

	return FALSE;
}