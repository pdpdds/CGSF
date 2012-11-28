#include "StdAfx.h"
#include "SFPlayerPlayReady.h"
#include "SFSendPacket.h"
#include "SFRoom.h"
#include "SFRoomManager.h"
#include "SFPlayer.h"

SFPlayerPlayReady::SFPlayerPlayReady(SFPlayer* pOwner, ePlayerState State)
: SFPlayerState(pOwner, State)
{
}

SFPlayerPlayReady::~SFPlayerPlayReady(void)
{
}

BOOL SFPlayerPlayReady::OnEnter()
{
	SFPlayer* pPlayer = GetOwner();
	_CharacterInfo* pInfo = pPlayer->GetCharacterInfo();
	pInfo->IsAlive = true;

	SFRoomManager* pManager = SFLogicEntry::GetLogicEntry()->GetRoomManager();
	SFRoom* pRoom = pManager->GetRoom(GetOwner()->GetRoomIndex());
	
	SendCreatePlayer(GetOwner(), pRoom);

	return TRUE;
}

BOOL SFPlayerPlayReady::OnLeave()
{
	return TRUE;
}

BOOL SFPlayerPlayReady::ProcessPacket( BasePacket* pPacket )
{
	return TRUE;
}

BOOL SFPlayerPlayReady::ProcessDBResult( SFMessage* pMessage )
{
	return TRUE;
}
