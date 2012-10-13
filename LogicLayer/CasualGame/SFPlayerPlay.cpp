#include "StdAfx.h"
#include "SFPlayerPlay.h"
#include "SFPlayer.h"
#include "SFRoom.h"
#include "SFRoomManager.h"
#include "GamePacketStructure.h"

SFPlayerPlay::SFPlayerPlay(SFPlayer* pOwner, ePlayerState State)
: SFPlayerState(pOwner, State)
{
}

SFPlayerPlay::~SFPlayerPlay(void)
{
}

BOOL SFPlayerPlay::OnEnter()
{
	SFPlayer* pPlayer = GetOwner();
	SendPlayStart(pPlayer);

	int RoomIndex = pPlayer->GetRoomIndex();

	SFRoomManager* pRoomManager = SFLogicEntry::GetLogicEntry()->GetRoomManager();
	SFRoom* pRoom = pRoomManager->GetRoom(RoomIndex);
	pRoom->ProcessUserRequest(pPlayer, CGSF::PlayScoreInfo);

	return TRUE;
}

BOOL SFPlayerPlay::OnLeave()
{
	SFPlayer* pPlayer = GetOwner();
	SendPlayEnd(pPlayer);

	return TRUE;
}

BOOL SFPlayerPlay::ProcessPacket( SFPacket* pPacket )
{
	SFPlayer* pPlayer = GetOwner();
	int RoomIndex = pPlayer->GetRoomIndex();

	SFRoomManager* pRoomManager = SFLogicEntry::GetLogicEntry()->GetRoomManager();
	SFRoom* pRoom = pRoomManager->GetRoom(RoomIndex);

	pRoom->ProcessUserRequest(pPlayer, pPacket);

	return TRUE;
}

