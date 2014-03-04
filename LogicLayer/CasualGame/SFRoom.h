#pragma once
#include "SFTeam.h"
#include "SFSubject.h"
#include "SFRoomState.h"
#include "SFP2PSys.h"
#include "SFGameStructure.h"

class SFPlayer;
class SFRoomFSM;

#define  MAX_ROOM_MEMBER 4

class SFRoom : public SFSubject
{
public:
	typedef std::map<int, SFPlayer*> RoomMemberMap;
public:
	SFRoom(int RoomIndex);
	virtual ~SFRoom(void);

	BOOL ProcessUserRequest(SFPlayer* pPlayer, BasePacket* pPacket);
	BOOL ProcessUserRequest(SFPlayer* pPlayer, int Msg);

	BOOL Open(SFPlayer* pPlayer);
	BOOL Close();

	BOOL Update(DWORD dwTickCount);

	BOOL OnEnter(SFPlayer* pPlayer);
	BOOL OnLeave(SFPlayer* pPlayer);

	int GetRoomIndex(){return m_RoomIndex;}
	int GetRoomState();

	BOOL CheckPlayerState(ePlayerState State);
	BOOL CheckLoadingComplete();
	BOOL ChangePlayerFSM(ePlayerState State);

	void SetRoomChief(SFPlayer* pPlayer){m_pRoomChief = pPlayer;}
	SFPlayer* GetRoomChief(){return m_pRoomChief;}

	BOOL ChangeTeam(SFPlayer* pPlayer);
	BOOL CanEnter(SFPlayer* pPlayer);

	BOOL AddTeamMember(SFPlayer* pPlayer);
	BOOL AddTeamMember(SFPlayer* pPlayer, eTeamType TeamType);
	BOOL DelTeamMember(SFPlayer* pPlayer);

	_GameModeInfo* GetGameModeInfo(){return &m_GameModeInfo;}

	SFPlayer* GetPlayer(std::string& szName);

	BOOL ChangeState(eRoomState State);

	SFTeam* SelectMyTeam(SFPlayer* pPlayer);

	RoomMemberMap& GetRoomMemberMap(){return m_RoomMemberMap;}

	BOOL BroadCast(BasePacket* pPacket);

	SFRoomFSM* GetRoomFSM(){return m_pRoomFSM;}

	SFP2PSys* GetP2PSys(){return &m_P2PSys;}

	std::string m_roomName;

protected:

private:
	SFTeam m_Team[TEAM_TYPE_MAX];
	int m_RoomIndex;
	
	SFPlayer* m_pRoomChief;

	SFRoomFSM* m_pRoomFSM;

	_GameModeInfo m_GameModeInfo;
	RoomMemberMap m_RoomMemberMap;
	SFP2PSys m_P2PSys;
};
