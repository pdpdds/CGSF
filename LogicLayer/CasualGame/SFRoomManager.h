#pragma once
#include "SFRoom.h"

#define MAX_ROOM_COUNT 100

class SFPlayerLobby;

class SFRoomManager
{
	typedef std::map<int, SFRoom*> EmptyRoomMap;
	typedef std::map<int, SFRoom*> ActiveRoomMap;

public:
	SFRoomManager(void);
	virtual ~SFRoomManager(void);

	BOOL Initialize();
	BOOL Update(int timerId);

	SFRoom* GetEmptyRoom();
	BOOL RecallRoom(SFRoom* pRoom);

	BOOL OnCreateRoom(SFPlayerLobby* pLobbyState, int GameMode);
	SFRoom* GetRoom(int RoomIndex);
	SFRoom* GetWaitRoom();

	BOOL SendRoomPage(SFPlayerLobby* pLobbyState, int PageIndex = 0);

	ActiveRoomMap& GetActiveRoomMap(){return m_ActiveRoomMap;}

protected:

private:
	EmptyRoomMap m_EmptyRoomMap;
	ActiveRoomMap m_ActiveRoomMap;

	SFRoom* m_RoomArray[MAX_ROOM_COUNT];
};
