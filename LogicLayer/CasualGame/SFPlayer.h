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
	void SetSerial(int serial){m_serial = serial;}

	BOOL ChangeState(ePlayerState State);
	ePlayerState GetPlayerState();
	BOOL ProcessPacket(BasePacket* pPacket);
	BOOL ProcessDBResult(SFMessage* pMessage);

	std::string& GetPlayerName(){return m_username;}
	std::string m_username;
	std::string m_password;

	int GetSerial(){return m_serial;}

	int GetRoomIndex(){return m_roomIndex;}
	void SetRoomIndex(int RoomIndex){m_roomIndex = RoomIndex;}

	_UserInfo* GetUserInfo(){return &m_userInfo;}

	void SetMyTeam(eTeamType Type){m_teamType = Type;}
	eTeamType GetMyTeam(){return m_teamType;}

	BOOL GetLoadingComplete(){return m_bLoadingComplete;}
	void SetLoadingComplete(BOOL bLoading){m_bLoadingComplete = bLoading;}

	void SetSpawnIndex(int Index){m_spawnIndex = Index;}
	int GetSpawnIndex(){return m_spawnIndex;}

	_CharacterInfo* GetCharacterInfo(){return &m_characterInfo;}

	void SetIPInfo(_PeerInfo& info){ m_info = info; }
	_PeerInfo& GetIPInfo(){ return m_info; }

protected:

private:
	SFPlayerFSM* m_pFSM;
	int m_serial;

	int m_roomIndex;
	_UserInfo m_userInfo;

	eTeamType m_teamType;

	BOOL m_bLoadingComplete;

	int m_spawnIndex;

	_CharacterInfo m_characterInfo;
	_PeerInfo m_info;
};
