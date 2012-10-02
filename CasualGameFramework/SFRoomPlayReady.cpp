#include "StdAfx.h"
#include "SFRoomPlayReady.h"
#include "SFRoom.h"
#include "SFPlayer.h"

SFRoomPlayReady::SFRoomPlayReady(SFRoom* pOwner, eRoomState State)
: SFRoomState(pOwner, State)
{
}

SFRoomPlayReady::~SFRoomPlayReady(void)
{
}

BOOL SFRoomPlayReady::OnEnter()
{
	SFRoom* pRoom = GetOwner();

	SFRoom::RoomMemberMap MemberMap = pRoom->GetRoomMemberMap();

	SFRoom::RoomMemberMap::iterator iter = MemberMap.begin();

	int Index = 1;
	for(;iter != MemberMap.end();iter++)
	{
		SFPlayer* pPlayer = iter->second;
		pPlayer->SetSpawnIndex(Index);
		Index++;
	}
	
	pRoom->ChangePlayerFSM(PLAYER_STATE_PLAYREADY);

	return TRUE;
}

BOOL SFRoomPlayReady::OnLeave()
{
	return TRUE;
}

BOOL SFRoomPlayReady::ProcessUserRequest( SFPlayer* pPlayer, SFPacket* pPacket )
{
	SFRoom* pRoom = GetOwner();

	/*switch(pPacket->GetPacketID())
	{
	case CGSF::ReportIP:
		{
			OnReportIP(pPlayer);
		}
		break;
	}*/

	return TRUE;
}

BOOL SFRoomPlayReady::OnReportIP( SFPlayer* pPlayer, SFPacket* pPacket )
{
	return TRUE;
}

