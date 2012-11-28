#pragma once
#include "SFPlayerState.h"
#include "CommonStructure.h"
#include "SFObserver.h"
#include "SFACEAllocator.h"
#include "PeerInfo.h"

class SFPlayerFSM;

class SFPlayer : public AceMemoryObject<SFPlayer>, public SFObserver
{
public:
	SFPlayer(void);
	virtual ~SFPlayer(void);

	virtual BOOL OnMessage(SFObject* pSender, SFMessage* pMessage) override;

	BOOL Initialize();
	BOOL Finally();
	BOOL Reset();
	void SetSerial(int Serial){m_Serial = Serial;}

	BOOL ChangeState(ePlayerState State);
	ePlayerState GetPlayerState();
	BOOL ProcessPacket(BasePacket* pPacket);
	BOOL ProcessDBResult(SFMessage* pMessage);

	std::string m_username;
	std::string m_password;

	int GetSerial(){return m_Serial;}

	int GetRoomIndex(){return m_RoomIndex;}
	void SetRoomIndex(int RoomIndex){m_RoomIndex = RoomIndex;}

	_UserInfo* GetUserInfo(){return &m_UserInfo;}

	void SetMyTeam(eTeamType Type){m_TeamType = Type;}
	eTeamType GetMyTeam(){return m_TeamType;}

	BOOL GetLoadingComplete(){return m_bLoadingComplete;}
	void SetLoadingComplete(BOOL bLoading){m_bLoadingComplete = bLoading;}

	void SetSpawnIndex(int Index){m_SpawnIndex = Index;}
	int GetSpawnIndex(){return m_SpawnIndex;}

	_CharacterInfo* GetCharacterInfo(){return &m_CharacterInfo;}

	void SetIPInfo(_PeerInfo& Info){m_Info = Info;}
	_PeerInfo& GetIPInfo(){return m_Info;}

protected:

private:
	SFPlayerFSM* m_pFSM;
	int m_Serial;

	int m_RoomIndex;
	_UserInfo m_UserInfo;

	eTeamType m_TeamType;

	BOOL m_bLoadingComplete;

	int m_SpawnIndex;

	_CharacterInfo m_CharacterInfo;
	_PeerInfo m_Info;
};
