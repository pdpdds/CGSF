#include "StdAfx.h"
#include "SFPlayerRoom.h"
#include "DBMsg.h"
#include "SFRoomManager.h"
#include "SFPlayer.h"

SFPlayerRoom::SFPlayerRoom(SFPlayer* pOwner, ePlayerState State)
: SFPlayerState(pOwner, State)
{
}

SFPlayerRoom::~SFPlayerRoom(void)
{
}

BOOL SFPlayerRoom::OnEnter()
{
	return TRUE;
}

BOOL SFPlayerRoom::OnLeave()
{
	return TRUE;
}

BOOL SFPlayerRoom::ProcessPacket( BasePacket* pPacket )
{
	SFPlayer* pPlayer = GetOwner();
	SFRoomManager* pRoomManager = SFLogicEntry::GetLogicEntry()->GetRoomManager();
	SFRoom* pRoom = pRoomManager->GetRoom(pPlayer->GetRoomIndex());

	if(pRoom == NULL)
		return FALSE;

	switch(pPacket->GetPacketID())
	{
	case CGSF::ChangeTeam:
	case CGSF::LeaveRoom:
	case CGSF::LoadingStart:
	case CGSF::ChatReq:
		{
			pRoom->ProcessUserRequest(pPlayer, pPacket);
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;

}

BOOL SFPlayerRoom::ProcessDBResult( SFMessage* pMessage )
{
	return TRUE;
}