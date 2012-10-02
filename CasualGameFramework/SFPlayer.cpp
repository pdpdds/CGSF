#include "StdAfx.h"
#include "SFPlayer.h"
#include "SFPlayerFSM.h"
#include "SFSendPacket.h"
#include "SFRoomManager.h"
#include "SFLogicEntry.h"

SFPlayer::SFPlayer(void)
: m_pFSM(NULL)
{
	Reset();
	Initialize();
}

SFPlayer::~SFPlayer(void)
{
}

BOOL SFPlayer::Initialize(void)
{
	m_pFSM = new SFPlayerFSM(this, PLAYER_STATE_NONE);

	return TRUE;
}

BOOL SFPlayer::Finally(void)
{
	if(m_pFSM)
	{
		delete m_pFSM;
		m_pFSM = NULL;
	}

	return TRUE;
}

BOOL SFPlayer::Reset(void)
{
	if(m_pFSM)
		m_pFSM->Reset();

	m_bLoadingComplete = FALSE;
	m_TeamType = TEAM_RED;

	return TRUE;
}

BOOL SFPlayer::ChangeState(ePlayerState State)
{
	return m_pFSM->ChangeState(State);
}

ePlayerState SFPlayer::GetPlayerState()
{
	return m_pFSM->GetPlayerState();
}

BOOL SFPlayer::ProcessPacket(SFPacket* pPacket)
{
	return m_pFSM->ProcessPacket(pPacket);
}

BOOL SFPlayer::ProcessDBResult(SFMessage* pMessage)
{
	return m_pFSM->ProcessDBResult(pMessage);
}

BOOL SFPlayer::OnMessage( SFObject* pSender, SFMessage* pMessage )
{
	switch(pMessage->GetCommand())
	{
	case CGSF::EnterTeamMember:
		{
			int PlayerIndex = -1;
			pMessage->ReadStart();
			*pMessage >> PlayerIndex;

			pMessage->ReadEnd();

			SendEnterTeamMember(this, PlayerIndex);
		}
		break;	
	case CGSF::LeaveTeamMember:
		{
			int PlayerIndex = -1;
			pMessage->ReadStart();
			*pMessage >> PlayerIndex;

			pMessage->ReadEnd();

			SendLeaveTeamMember(this, PlayerIndex);
		}
		break;
	
	case CGSF::MSG_DESTROY_PLAYER:
		{
			int PlayerIndex = -1;
			pMessage->ReadStart();
			*pMessage >> PlayerIndex;

			pMessage->ReadEnd();

			SendDestroyPlayer(this, PlayerIndex);
		}
		break;
	case CGSF::PeerList:
		{
			SFRoomManager* pManager = SFLogicEntry::GetLogicEntry()->GetRoomManager();
			SFRoom* pRoom = pManager->GetRoom(GetRoomIndex());

			SendPeerInfo(this, pRoom);
		}
		break;
	case CGSF::DeletePeer:
		{
			int PlayerIndex = -1;
			pMessage->ReadStart();
			*pMessage >> PlayerIndex;

			pMessage->ReadEnd();

			SendDeletePeer(this, PlayerIndex);
		}
		break;

	}

	return TRUE;
}