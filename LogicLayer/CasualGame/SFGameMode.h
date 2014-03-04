#pragma once
#include "SFDispatch.h"

class SFPlayer;
class SFRoomPlay;
class BasePacket;
class SFPlayer;

class SFGameMode
{
public:
	SFGameMode(int GameMode);
	virtual ~SFGameMode(void);

	void SetOwner(SFRoomPlay* pOwner){m_pOwner = pOwner;}
	virtual SFGameMode* Clone() = 0;

	virtual BOOL OnEnter(int GameMode) {return FALSE;}
	virtual BOOL Onleave() {return FALSE;}
	virtual BOOL ProcessUserRequest(SFPlayer* pPlayer, BasePacket* pPacket) {return FALSE;}
	virtual BOOL ProcessUserRequest(SFPlayer* pPlayer, int Msg) {return FALSE;}
	virtual BOOL Update(DWORD dwTickcount) {return FALSE;}

	SFRoomPlay* GetOwner(){return m_pOwner;}
	int GetGameMode(){return m_GameMode;}

	void Reset();

protected:
	SFDispatch_2<USHORT, std::tr1::function<BOOL(SFPlayer*, BasePacket*)>, SFPlayer*, BasePacket*> m_DispatchingSystem;

private:
	SFRoomPlay* m_pOwner;
	int m_GameMode;
};
