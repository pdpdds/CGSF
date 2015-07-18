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

#define MAX_ROOM_COUNT_PER_PAGE 10
BOOL SFRoomManager::SendRoomPage( SFPlayerLobby* pLobbyState, int PageIndex /*= 0*/ )
{
	SFPlayer* pOwner = pLobbyState->GetOwner();
	int roomCnt = m_ActiveRoomMap.size();

	//if(roomCnt == 0)
		//return TRUE;

	SFProtobufPacket<SFPacketStore::RoomList> roomList = SFProtobufPacket<SFPacketStore::RoomList>(CGSF::RoomList);
	roomList.SetSerial(pOwner->GetSerial());

	if(m_ActiveRoomMap.size() <= MAX_ROOM_COUNT_PER_PAGE)
	{
		ActiveRoomMap::iterator iter = m_ActiveRoomMap.begin();

		for(;iter != m_ActiveRoomMap.end(); iter++)
		{
			SFPacketStore::RoomList::RoomInfo* pRoomInfo = roomList.GetData().add_info();
			SFRoom* pRoom = iter->second;

			pRoomInfo->set_roomindex(pRoom->GetRoomIndex());

			std::string roomName = "GameRoom";
			roomName += std::to_string(pRoom->GetRoomIndex());

			pRoomInfo->set_roomname(roomName);

			pRoomInfo->set_roomchief(pRoom->GetRoomChief()->GetPlayerName());
			pRoomInfo->set_usercount(pRoom->GetRoomMemberMap().size());
			pRoomInfo->set_maxusercount(4);
			pRoomInfo->set_needpassword(0);
			pRoomInfo->set_gamemode(pRoom->GetGameModeInfo()->GameMode);
		}
	}

	SFLogicEntry::GetLogicEntry()->SendRequest(&roomList);

	return TRUE;
}

BOOL SFRoomManager::Update(int timerID)
{
	for (auto& iter : m_ActiveRoomMap)
	{
		SFRoom* pRoom = iter.second;
		pRoom->Update(timerID);
		//if (thread.joinable())
			//thread.join();
	}

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

SFRoom* SFRoomManager::GetWaitRoom()
{

	for (int i = 0; i < MAX_ROOM_COUNT;i++)
	{
		SFRoom* pRoom = m_RoomArray[i];

		if (pRoom->GetRoomState() == ROOM_STATE_WAIT)
		{
			return pRoom;
		}		
	}
	

	return NULL;
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