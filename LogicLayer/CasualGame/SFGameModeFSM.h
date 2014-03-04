#pragma once
#include <map>
#include "SFGameMode.h"

class SFRoomPlay;

class SFGameModeFSM : public SFGameMode
{
	friend class SFRoomPlay;
	typedef std::map<int, SFGameMode*> GameModeMap;

public:
	SFGameModeFSM(int Mode);
	virtual ~SFGameModeFSM(void);

	virtual SFGameMode* Clone(){return FALSE;} //메소드를 딴 클래스로 분리하는 것이 좋음

	BOOL Initialize(SFRoomPlay* pOwner);

	virtual BOOL OnEnter(int gameMode) override;
	virtual BOOL Onleave() override;

	virtual BOOL Update(DWORD dwTickcount) override;

protected:
	BOOL AddGameMode(SFRoomPlay* pOwner);
	BOOL ProcessUserRequest(SFPlayer* pPlayer, BasePacket* pPacket) override;
	BOOL ProcessUserRequest(SFPlayer* pPlayer, int Msg) override;
	BOOL SetGameMode(int gameMode);

private:
	GameModeMap m_GameModeMap;
	SFGameMode* m_pCurrentGameMode;
};
