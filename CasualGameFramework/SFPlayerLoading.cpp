#include "StdAfx.h"
#include "SFPlayerLoading.h"
#include "SFPlayer.h"
#include "SFSendPacket.h"
#include "SFRoomManager.h"

SFPlayerLoading::SFPlayerLoading(SFPlayer* pOwner, ePlayerState State)
: SFPlayerState(pOwner, State)
{
}

SFPlayerLoading::~SFPlayerLoading(void)
{
}

BOOL SFPlayerLoading::OnEnter()
{
	SFPlayer* pPlayer = GetOwner();
	pPlayer->SetLoadingComplete(FALSE);

	SendLoadingStart(pPlayer);
	return TRUE;
}

BOOL SFPlayerLoading::OnLeave()
{
	SFPlayer* pPlayer = GetOwner();
	return TRUE;
}

BOOL SFPlayerLoading::ProcessPacket( SFPacket* pPacket )
{
	switch(pPacket->GetPacketID())
	{
	case CGSF::LoadingComplete:
		{
			OnLoadingComplete(pPacket);
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

BOOL SFPlayerLoading::ProcessDBResult( SFMessage* pMessage )
{
	return TRUE;
}

BOOL SFPlayerLoading::OnLoadingComplete( SFPacket* pPacket )
{
	SFPlayer* pPlayer = GetOwner();
	SFRoomManager* pManager = SFLogicEntry::GetLogicEntry()->GetRoomManager();

	SFRoom* pRoom = pManager->GetRoom(pPlayer->GetRoomIndex());

	pPlayer->SetLoadingComplete(TRUE);

	pRoom->ProcessUserRequest(pPlayer, pPacket);

	return TRUE;
}