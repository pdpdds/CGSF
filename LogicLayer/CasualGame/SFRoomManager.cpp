#include "StdAfx.h"
#include "SFRoomManager.h"
#include "SFPlayerLobby.h"
#include "SFPlayer.h"

SFRoomManager::SFRoomManager(void)
{
}

SFRoomManager::~SFRoomManager(void)
{
}

BOOL SFRoomManager::SendRoomPage( SFPlayerLobby* pLobbyState, int PageIndex /*= 0*/ )
{
	return TRUE;
}

SFRoom* SFRoomManager::GetRoom( int RoomIndex )
{
	if(RoomIndex < 0 || RoomIndex >= MAX_ROOM_COUNT)
	{
		SFASSERT(0);
		return NULL;
	}

	SFRoom* pRoom = m_RoomArray[RoomIndex];

	if(pRoom->GetRoomState() == ROOM_STATE_NONE)
	{
		return NULL;
	}

	return pRoom;
}

BOOL SFRoomManager::OnCreateRoom( SFPlayerLobby* pLobbyState, int GameMode )
{
	SFRoom* pRoom = GetEmptyRoom();
	SFPlayer* pPlayer = pLobbyState->GetOwner();

	if(NULL == pRoom)
		return FALSE;

	SFASSERT(pRoom->GetRoomState() == ROOM_STATE_NONE);

	pRoom->Open(pPlayer);
	pRoom->ChangeState(ROOM_STATE_WAIT);
	pPlayer->SetRoomIndex(pRoom->GetRoomIndex());

	_GameModeInfo* pInfo = pRoom->GetGameModeInfo();
	pInfo->SetGameMode(GameMode);

	m_ActiveRoomMap.insert(std::make_pair(pRoom->GetRoomIndex(), pRoom));

	return TRUE;
}

SFRoom* SFRoomManager::GetEmptyRoom()
{
	EmptyRoomMap::iterator iter = m_EmptyRoomMap.begin();

	if(iter == m_EmptyRoomMap.end())
		return NULL;

	SFRoom* pRoom = iter->second;

	SFASSERT(pRoom->GetRoomState() == ROOM_STATE_NONE);

	m_EmptyRoomMap.erase(iter);

	return pRoom;
}

BOOL SFRoomManager::Initialize()
{
	for(int i = 0; i < MAX_ROOM_COUNT; i++)
	{
		m_RoomArray[i] = new SFRoom(i);

		m_EmptyRoomMap.insert(make_pair(i, m_RoomArray[i]));
	}

	return TRUE;
}

BOOL SFRoomManager::RecallRoom( SFRoom* pRoom )
{
	pRoom->ChangeState(ROOM_STATE_NONE);

	m_EmptyRoomMap.insert(make_pair(pRoom->GetRoomIndex(), pRoom));
	m_ActiveRoomMap.erase(pRoom->GetRoomIndex());

	return TRUE;
}