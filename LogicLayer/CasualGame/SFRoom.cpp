#include "StdAfx.h"
#include "SFRoom.h"
#include "SFRoomFSM.h"
#include "SFObserver.h"
#include "SFTeam.h"
#include "SFPlayer.h"
#include "SFRoomManager.h"
#include "SFSendPacket.h"

SFRoom::SFRoom(int RoomIndex)
: m_RoomIndex(RoomIndex)
{
	m_pRoomFSM = new SFRoomFSM(this, ROOM_STATE_NONE);

	for(int i = 0; i < TEAM_TYPE_MAX; i++)
	{
		m_Team[i].SetTeamType(i);
	}
}

SFRoom::~SFRoom(void)
{
}

int SFRoom::GetRoomState()
{
	return m_pRoomFSM->GetRoomState();
}

BOOL SFRoom::ProcessUserRequest( SFPlayer* pPlayer, BasePacket* pPacket )
{
	return m_pRoomFSM->ProcessUserRequest(pPlayer, pPacket);
}

BOOL SFRoom::ProcessUserRequest( SFPlayer* pPlayer, int Msg )
{
	return m_pRoomFSM->ProcessUserRequest(pPlayer, Msg);
}

BOOL SFRoom::ChangeState( eRoomState State )
{
	return m_pRoomFSM->ChangeState(State);
}

BOOL SFRoom::Open( SFPlayer* pPlayer )
{
	AddObserver((SFObserver*)pPlayer);
	SetRoomChief(pPlayer);
	AddTeamMember(pPlayer);
	m_RoomMemberMap.insert(make_pair(pPlayer->GetSerial(), pPlayer));

	return TRUE;
}

BOOL SFRoom::AddTeamMember( SFPlayer* pPlayer)
{
	SFTeam* pTeam = SelectMyTeam(pPlayer);
	pPlayer->SetMyTeam(pTeam->GetTeamType());
	pTeam->AddMember(pPlayer);
	
	//SendTeamInfo(pPlayer, pTeam);
	return TRUE;
}

BOOL SFRoom::ChangePlayerFSM( ePlayerState State )
{
	for(int i = 0; i < TEAM_TYPE_MAX; i++)
	{
		TeamMemberMap& MemberMap = m_Team[i].GetMemberMap();

		TeamMemberMap::iterator iter = MemberMap.begin();

		for(iter; iter != MemberMap.end(); iter++)
		{
			SFPlayer* pPlayer = iter->second;

			pPlayer->ChangeState(State);
		}
	}

	return TRUE;
}

BOOL SFRoom::OnEnter( SFPlayer* pPlayer )
{
	if (MAX_ROOM_MEMBER <= GetObserverCount())
		return FALSE;
	
	if(FALSE == AddObserver(pPlayer))
		return FALSE;

	AddTeamMember(pPlayer);

	pPlayer->SetRoomIndex(GetRoomIndex());

	SFMessage Message;
	Message.Initialize(CGSF::EnterTeamMember);
	Message << pPlayer->GetSerial();

	PropagateMessage(&Message);

	m_RoomMemberMap.insert(make_pair(pPlayer->GetSerial(), pPlayer));

	return TRUE;
}

BOOL SFRoom::OnLeave( SFPlayer* pPlayer )
{
	m_RoomMemberMap.erase(pPlayer->GetSerial());

	DelObserver(pPlayer);

	SFMessage Message;
	Message.Initialize(CGSF::LeaveTeamMember);
	Message << pPlayer->GetSerial();

	PropagateMessage(&Message);

	DelTeamMember(pPlayer);

	if(0 == GetObserverCount())
	{
		SFRoomManager* pManager = SFLogicEntry::GetLogicEntry()->GetRoomManager();
		pManager->RecallRoom(this);
	}
	else
	{
		if(GetRoomChief() == pPlayer)
		{
			RoomMemberMap::iterator iter = m_RoomMemberMap.begin();

			if(iter == m_RoomMemberMap.end())
			{
				SFASSERT(0);
				return FALSE;
			}

			SetRoomChief(iter->second);
		}

	}

	return TRUE;
}

BOOL SFRoom::ChangeTeam( SFPlayer* pPlayer )
{
	eTeamType TeamType = pPlayer->GetMyTeam();
	DelTeamMember(pPlayer);

	TeamType = (eTeamType)(TeamType + 1);

	if(TeamType >= TEAM_TYPE_MAX)
		TeamType = TEAM_RED;

	AddTeamMember(pPlayer, TeamType);

	return TRUE;
}

BOOL SFRoom::CheckPlayerState( ePlayerState State )
{
	for(int i = 0; i < TEAM_TYPE_MAX; i++)
	{
		TeamMemberMap& MemberMap = m_Team[i].GetMemberMap();

		TeamMemberMap::iterator iter = MemberMap.begin();

		for(iter; iter != MemberMap.end(); iter++)
		{
			SFPlayer* pPlayer = iter->second;

			if(pPlayer->GetPlayerState() != State)
				return FALSE;
		}
	}

	return TRUE;
}

BOOL SFRoom::DelTeamMember( SFPlayer* pPlayer )
{
	m_Team[pPlayer->GetMyTeam()].DelMember(pPlayer);

	return TRUE;
}

SFTeam* SFRoom::SelectMyTeam(SFPlayer* pPlayer)
{
	if(m_Team[TEAM_RED].GetMemberCount() > m_Team[TEAM_BLUE].GetMemberCount())
	{
		return &m_Team[TEAM_BLUE];
	}

	return &m_Team[TEAM_RED];
}

BOOL SFRoom::AddTeamMember( SFPlayer* pPlayer, eTeamType TeamType )
{
	pPlayer->SetMyTeam(TeamType);
	m_Team[TeamType].AddMember((pPlayer));

	//SendTeamInfo(pPlayer, &m_Team[TEAM_RED]);
	//SendTeamInfo(pPlayer, &m_Team[TEAM_blue]);

	return TRUE;
}

BOOL SFRoom::CheckLoadingComplete()
{
	for(int i = 0; i < TEAM_TYPE_MAX; i++)
	{
		TeamMemberMap& MemberMap = m_Team[i].GetMemberMap();

		TeamMemberMap::iterator iter = MemberMap.begin();

		for(iter; iter != MemberMap.end(); iter++)
		{
			SFPlayer* pPlayer = iter->second;

			if(pPlayer->GetLoadingComplete() != TRUE)
				return FALSE;
		}
	}

	return TRUE;
	
}

BOOL SFRoom::CanEnter( SFPlayer* pPlayer )
{
	return FALSE;
}

BOOL SFRoom::BroadCast( BasePacket* pPacket )
{
	for(int i = 0; i < TEAM_TYPE_MAX; i++)
	{
		TeamMemberMap& MemberMap = m_Team[i].GetMemberMap();

		TeamMemberMap::iterator iter = MemberMap.begin();

		for(iter; iter != MemberMap.end(); iter++)
		{
			SFPlayer* pPlayer = iter->second;

			SendToClient(pPlayer, pPacket);
		}
	}
	return TRUE;
}