#include "StdAfx.h"
#include "SFRoomFSM.h"
#include "SFRoomNone.h"
#include "SFRoomWait.h"
#include "SFRoomLoading.h"
#include "SFRoomPlayReady.h"
#include "SFRoomPlay.h"
#include "SFRoomPlayEnd.h"

SFRoomFSM::SFRoomFSM( SFRoom* pOwner, eRoomState State )
: SFRoomState(pOwner, State)
, m_pCurrentState(NULL)
{
	AddState(ROOM_STATE_NONE);
	AddState(ROOM_STATE_WAIT);
	AddState(ROOM_STATE_LOADING);
	AddState(ROOM_STATE_PLAYREADY);
	AddState(ROOM_STATE_PLAY);
	AddState(ROOM_STATE_PLAYEND);

	RoomStateMap::iterator iter = m_RoomStateMap.find(ROOM_STATE_NONE);
	m_pCurrentState = iter->second;
}

SFRoomFSM::~SFRoomFSM(void)
{
}

BOOL SFRoomFSM::ChangeState( eRoomState State )
{
	if(m_pCurrentState->GetRoomState() == State)
	{
		SFASSERT(0);
		return FALSE;
	}

	RoomStateMap::iterator iter = m_RoomStateMap.find(State);

	if(iter == m_RoomStateMap.end())
	{
		SFASSERT(0);
		return FALSE;
	}

	m_pCurrentState->OnLeave();

	m_pCurrentState = iter->second;

	m_pCurrentState->OnEnter();

	return TRUE;
}

BOOL SFRoomFSM::AddState( eRoomState State )
{
	switch(State)
	{
	case ROOM_STATE_NONE:
		{
			SFRoomState* pState = new SFRoomNone(GetOwner(), ROOM_STATE_NONE);
			m_RoomStateMap.insert(make_pair(ROOM_STATE_NONE, pState));
		}
		break;
	case ROOM_STATE_WAIT:
		{
			SFRoomState* pState = new SFRoomWait(GetOwner(), ROOM_STATE_WAIT);
			m_RoomStateMap.insert(make_pair(ROOM_STATE_WAIT, pState));
		}
		break;
	case ROOM_STATE_LOADING:
		{
			SFRoomState* pState = new SFRoomLoading(GetOwner(), ROOM_STATE_LOADING);
			m_RoomStateMap.insert(make_pair(ROOM_STATE_LOADING, pState));
		}
		break;
	case ROOM_STATE_PLAYREADY:
		{
			SFRoomState* pState = new SFRoomPlayReady(GetOwner(), ROOM_STATE_PLAYREADY);
			m_RoomStateMap.insert(make_pair(ROOM_STATE_PLAYREADY, pState));
		}
		break;
	case ROOM_STATE_PLAY:
		{
			SFRoomState* pState = new SFRoomPlay(GetOwner(), ROOM_STATE_PLAY);
			m_RoomStateMap.insert(make_pair(ROOM_STATE_PLAY, pState));
		}
		break;
	case ROOM_STATE_PLAYEND:
		{
			SFRoomState* pState = new SFRoomPlayEnd(GetOwner(), ROOM_STATE_PLAYEND);
			m_RoomStateMap.insert(make_pair(ROOM_STATE_PLAYEND, pState));
		}
		break;
	}

	return TRUE;
}

BOOL SFRoomFSM::ProcessUserRequest( SFPlayer* pPlayer, BasePacket* pPacket )
{
	if(m_pCurrentState)
		return m_pCurrentState->ProcessUserRequest(pPlayer, pPacket );

	return FALSE;
}

BOOL SFRoomFSM::ProcessUserRequest( SFPlayer* pPlayer, int Msg )
{
	if(m_pCurrentState)
		return m_pCurrentState->ProcessUserRequest(pPlayer, Msg );

	return FALSE;
}

BOOL SFRoomFSM::Update( DWORD timerId )
{
	if(m_pCurrentState)
		return m_pCurrentState->Update(timerId);

	return FALSE;
}

BOOL SFRoomFSM::Reset()
{
	RoomStateMap::iterator iter = m_RoomStateMap.find(ROOM_STATE_NONE);

	SFASSERT(iter != m_RoomStateMap.end());

	m_pCurrentState = iter->second;

	return TRUE;
}

BOOL SFRoomFSM::OnEnterRoom( SFPlayer* pPlayer )
{
	m_pCurrentState->OnEnterRoom(pPlayer);

	return TRUE;
}

BOOL SFRoomFSM::OnLeaveRoom( SFPlayer* pPlayer )
{
	m_pCurrentState->OnLeaveRoom(pPlayer);

	return TRUE;
}