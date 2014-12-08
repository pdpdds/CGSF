#pragma once
#include "SFGameMode.h"

class SGManager;

class SGBattle : public SFGameMode
{
public:
	SGBattle(int Mode);
	virtual ~SGBattle(void);

	virtual SFGameMode* Clone()
	{
		return new SGBattle(GetGameMode());
	}

////////////////////////////////////////////////////////////
//System Method
////////////////////////////////////////////////////////////
	virtual BOOL OnEnter(int gameMode) override;
	virtual BOOL Onleave() override;
	virtual BOOL Update(DWORD timerId) override;
	BOOL ProcessUserRequest(SFPlayer* pPlayer, BasePacket* pPacket) override;
	BOOL ProcessUserRequest(SFPlayer* pPlayer, int msg) override;
	BOOL BroadCast(BasePacket& pPacket);


////////////////////////////////////////////////////////////
//User Directive
////////////////////////////////////////////////////////////
	BOOL OnTurnPass( SFPlayer* pPlayer, BasePacket* pPacket);
	BOOL OnCardSubmit( SFPlayer* pPlayer, BasePacket* pPacket);

////////////////////////////////////////////////////////////
//Send Packet
////////////////////////////////////////////////////////////
	BOOL SendPlayerID();
	void SendCurrentTurn();
	void SendUserDie(int serial);
	void SendTurnPass(int serial);
	void SendCardSubmit(int serial, SCardInfo& cardInfo);
	void SendTableUpdate();
	BOOL SendInitCardCount();
	BOOL SendMyCardInfo();
	void SendWinner();

////////////////////////////////////////////////////////////
//Game Logic
////////////////////////////////////////////////////////////
	void ResetGame();
	BOOL ProcessTurnPass(int currentTurnUser);

private:
	SGManager *m_pSevenGameManager;
	int m_nGameState;

	DWORD m_dwLastTickCount;
	DWORD m_dwLastPlayTickCount;
};

