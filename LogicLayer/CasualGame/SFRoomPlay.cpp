#include "StdAfx.h"
#include "SFRoomPlay.h"
#include "SFGameModeFSM.h"
#include "SFRoom.h"
#include "SFPlayer.h"
#include "SFSendPacket.h"
#include "SFP2PSys.h"

SFRoomPlay::SFRoomPlay( SFRoom* pOwner, eRoomState State )
: SFRoomState(pOwner, State)
, m_bGameEnd(FALSE)
{
	m_pGameModeFSM = new SFGameModeFSM(-1);
	m_pGameModeFSM->Initialize(this);

	m_Dispatch.RegisterMessage(CGSF::EnterRoom, std::tr1::bind(&SFRoomPlay::OnEnterRoom, this, std::tr1::placeholders::_1));
	m_Dispatch.RegisterMessage(CGSF::LeaveRoom, std::tr1::bind(&SFRoomPlay::OnLeaveRoom, this, std::tr1::placeholders::_1));
	m_DispatchingSystem.RegisterMessage(CGSF::LoadingComplete, std::tr1::bind(&SFRoomPlay::OnLoadingComplete, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2));
	//m_DispatchingSystem.RegisterMessage(CGSF::ChangeTeam, std::tr1::bind(&SFRoomPlay::OnChat, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2));
}

SFRoomPlay::~SFRoomPlay(void)
{
}

BOOL SFRoomPlay::OnEnter()
{
	SFRoom* pRoom = GetOwner();

	_GameModeInfo* pInfo = pRoom->GetGameModeInfo();

	m_pGameModeFSM->SetGameMode(pInfo->GameMode);

	pRoom->ChangePlayerFSM(PLAYER_STATE_PLAY);

	if(FALSE == m_pGameModeFSM->OnEnter(pInfo->GameMode))
	{
		SFASSERT(0);
		return FALSE;
	}

	SFMessage Message;
	Message.Initialize(CGSF::PeerList);
	pRoom->PropagateMessage(&Message);

	return TRUE;
}

BOOL SFRoomPlay::OnLeave()
{
	SFRoom* pRoom = GetOwner();

	m_pGameModeFSM->Onleave();

	pRoom->ChangePlayerFSM(PLAYER_STATE_PLAYEND);

	return TRUE;
}

BOOL SFRoomPlay::Update(DWORD timerId)
{
	return m_pGameModeFSM->Update(timerId);

}

BOOL SFRoomPlay::OnEnterRoom( SFPlayer* pPlayer )
{
	SFRoom* pRoom = GetOwner();

	if(TRUE == pRoom->OnEnter(pPlayer))
	{
		int SpawnIndex = (rand() % MAX_ROOM_MEMBER) + 1; 
		pPlayer->SetSpawnIndex(SpawnIndex);
		pPlayer->ChangeState(PLAYER_STATE_LOADING);

		return TRUE;
	}

	return FALSE;
}

BOOL SFRoomPlay::OnLeaveRoom( SFPlayer* pPlayer )
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

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

BOOL SFRoomPlay::ProcessUserRequest( SFPlayer* pPlayer, BasePacket* pPacket )
{
	if(FALSE == m_DispatchingSystem.HandleMessage(pPacket->GetPacketID(), pPlayer, pPacket))
		if(FALSE == m_Dispatch.HandleMessage(pPacket->GetPacketID(), pPlayer))
			return m_pGameModeFSM->ProcessUserRequest(pPlayer, pPacket);

	return TRUE;
}

BOOL SFRoomPlay::ProcessUserRequest( SFPlayer* pPlayer, int Msg )
{
	m_pGameModeFSM->ProcessUserRequest(pPlayer, Msg);

	return TRUE;
}

BOOL SFRoomPlay::OnLoadingComplete( SFPlayer* pPlayer, BasePacket* pPacket )
{
	SFRoom* pOwner = GetOwner();

	SFProtobufPacket<SFPacketStore::MSG_CREATE_PLAYER> MsgCreatePlayer(CGSF::MSG_CREATE_PLAYER);
	MsgCreatePlayer.GetData().set_serial(pPlayer->GetSerial());
	MsgCreatePlayer.GetData().set_spawnindex(pPlayer->GetSpawnIndex());

	SFRoom::RoomMemberMap MemberMap = pOwner->GetRoomMemberMap();
	SFRoom::RoomMemberMap::iterator iter = MemberMap.begin();

	for(;iter != MemberMap.end(); iter++)
	{
		SFPlayer* pTarget = iter->second;

		if(pTarget == pPlayer)
			continue;
		SendToClient(pTarget, &MsgCreatePlayer);
	}

	pPlayer->ChangeState(PLAYER_STATE_PLAY);


	return TRUE;
}

