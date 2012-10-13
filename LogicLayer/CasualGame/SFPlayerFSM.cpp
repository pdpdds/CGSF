#include "StdAfx.h"
#include "SFPlayerFSM.h"
#include "SFPlayerNone.h"
#include "SFPlayerInit.h"
#include "SFPlayerLogin.h"
#include "SFPlayerLobby.h"
#include "SFPlayerRoom.h"
#include "SFPlayerLoading.h"
#include "SFPlayerPlayReady.h"
#include "SFPlayerPlay.h"
#include "SFPlayerPlayEnd.h"


SFPlayerFSM::SFPlayerFSM(SFPlayer* pOwner, ePlayerState State)
: SFPlayerState(pOwner, State)
{
	AddState(PLAYER_STATE_NONE);
	AddState(PLAYER_STATE_INIT);
	AddState(PLAYER_STATE_LOGIN);
	AddState(PLAYER_STATE_LOBBY);
	AddState(PLAYER_STATE_ROOM);
	AddState(PLAYER_STATE_LOADING);
	AddState(PLAYER_STATE_PLAYREADY);
	AddState(PLAYER_STATE_PLAY);
	AddState(PLAYER_STATE_PLAYEND);
}

SFPlayerFSM::~SFPlayerFSM(void)
{
}

BOOL SFPlayerFSM::Reset()
{
	PlayerStateMap::iterator iter = m_PlayerStateMap.find(PLAYER_STATE_NONE);

//	SFASSERT(iter != m_PlayerStateMap.end());
	
	m_pCurrentState = iter->second;

	return TRUE;
}

BOOL SFPlayerFSM::AddState(ePlayerState State)
{
	switch(State)
	{
	case PLAYER_STATE_NONE:
		{
			SFPlayerState* pState = new SFPlayerNone(GetOwner(), PLAYER_STATE_NONE);
			m_PlayerStateMap.insert(PlayerStateMap::value_type(PLAYER_STATE_NONE, pState));
		}
		break;
	case PLAYER_STATE_INIT:
		{
			SFPlayerState* pState = new SFPlayerInit(GetOwner(), PLAYER_STATE_INIT);
			m_PlayerStateMap.insert(PlayerStateMap::value_type(PLAYER_STATE_INIT, pState));
		}
		break;
	case PLAYER_STATE_LOGIN:
		{
			SFPlayerState* pState = new SFPlayerLogin(GetOwner(), PLAYER_STATE_LOGIN);
			m_PlayerStateMap.insert(PlayerStateMap::value_type(PLAYER_STATE_LOGIN, pState));
		}
		break;
	case PLAYER_STATE_LOBBY:
		{
			SFPlayerState* pState = new SFPlayerLobby(GetOwner(), PLAYER_STATE_LOBBY);
			m_PlayerStateMap.insert(PlayerStateMap::value_type(PLAYER_STATE_LOBBY, pState));
		}
		break;
	case PLAYER_STATE_ROOM:
		{
			SFPlayerState* pState = new SFPlayerRoom(GetOwner(), PLAYER_STATE_ROOM);
			m_PlayerStateMap.insert(PlayerStateMap::value_type(PLAYER_STATE_ROOM, pState));
		}
		break;
	case PLAYER_STATE_LOADING:
		{
			SFPlayerState* pState = new SFPlayerLoading(GetOwner(), PLAYER_STATE_LOADING);
			m_PlayerStateMap.insert(PlayerStateMap::value_type(PLAYER_STATE_LOADING, pState));
		}
		break;
	case PLAYER_STATE_PLAYREADY:
		{
			SFPlayerState* pState = new SFPlayerPlayReady(GetOwner(), PLAYER_STATE_PLAYREADY);
			m_PlayerStateMap.insert(PlayerStateMap::value_type(PLAYER_STATE_PLAYREADY, pState));
		}
		break;
	case PLAYER_STATE_PLAY:
		{
			SFPlayerState* pState = new SFPlayerPlay(GetOwner(), PLAYER_STATE_PLAY);
			m_PlayerStateMap.insert(PlayerStateMap::value_type(PLAYER_STATE_PLAY, pState));
		}
		break;
	case PLAYER_STATE_PLAYEND:
		{
			SFPlayerState* pState = new SFPlayerPlayEnd(GetOwner(), PLAYER_STATE_PLAYEND);
			m_PlayerStateMap.insert(PlayerStateMap::value_type(PLAYER_STATE_PLAYEND, pState));
		}
		break;
	default:
		return FALSE;

	}

	return TRUE;
}

BOOL SFPlayerFSM::ChangeState(ePlayerState State)
{
	if(m_pCurrentState->GetPlayerState() == State)
	{
		//SFASSERT(0);
		return FALSE;
	}

	PlayerStateMap::iterator iter = m_PlayerStateMap.find(State);

	if(iter == m_PlayerStateMap.end())
	{
		//SFASSERT(0);
		return FALSE;
	}

	m_pCurrentState->OnLeave();

	m_pCurrentState = iter->second;

	m_pCurrentState->OnEnter();

	return TRUE;
}

ePlayerState SFPlayerFSM::GetPlayerState()
{
	return m_pCurrentState->GetPlayerState();
}

BOOL SFPlayerFSM::ProcessPacket(SFPacket* pPacket)
{
	return m_pCurrentState->ProcessPacket(pPacket);
}

BOOL SFPlayerFSM::ProcessDBResult(SFMessage* pMessage)
{
	return m_pCurrentState->ProcessDBResult(pMessage);
}