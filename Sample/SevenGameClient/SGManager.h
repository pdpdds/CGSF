#pragma once
#include "Define.h"

class SGUser;
class SGTable;
class CSelectAlgorithm;
class CGameView;
class CGameDoc;
class SGStateManager;

class SGManager  
{
public:
	SGManager();
	virtual ~SGManager();

	void AllocateObjcet(int iUserNum, int iPassTicket);
    void InitializeData();
  
	void SendUserCardToTable(SGUser *pUser);		
	SGTable *GetTable(){return m_pTable;};
	SGUser * FindUser(int iUserID);
	void AddUser(int userSerial);
	void RemoveAllUser();
	
	int GetUserNum(){return m_iUserNum;};
	BOOL CheckGameEnd();

	BOOL CheckPassTicket(int iPosx,int iPosy);
	BOOL MouseInPassButton(int iPosx,int iPosy);

	SGUser *GetEffectUser(){return m_pEffectUser;}
	void SetEffectUser(SGUser *pUser){m_pEffectUser = pUser;}

	BOOL ChangeState(eSGState state);

	void ReArrangeUserCard(SGUser* pUser,int &iCount,int iCardNum);
	void UpdateTableState(SCardInfo* pInfo);

	float GetCardEffectTime(){return m_fCardEffectTime;}
	void SetCardEffectTime(float fTime){m_fCardEffectTime = fTime;}

	SPRITE_CARD_INFO* GetUserCard(){return m_CARD;}

	SCardInfo* GetSpriteCard(){return &m_SelectedCard;}
	VOID SetSpriteCard(SCardInfo& Card){m_SelectedCard = Card;}
	CARD_EFFECT_INFO* GetCardEffectInfo(){return &m_CardEffectInfo;}
	void SetCardEffectInfo(SCardInfo* pInfo);
	BOOL GetCardImageIndex(SCardInfo* pInfo, int& iCol, int& iRow);

	BOOL OnRender(float fElapsedTime);
	BOOL OnMessage(int iX, int iY);
	BOOL UserProcessGame(int iPosX, int iPosY, SCardInfo& CardInfo);

	int GetNextUserID(int currentUserID);
	void SetMyID(int PlayerIndex){m_MyID = PlayerIndex;}
	int GetMyID(){return m_MyID;}

	int m_MyID;
	BOOL m_bIsVictory;
	SGTable *m_pTable;
	std::vector<SGUser *>  m_userlist;
	std::vector<int> m_vecActivePlayer;
	std::vector<int> m_vecDisplayOrder;
	int m_iCurrentTurn ;

private:

	int m_iUserNum;
	int m_iPassTicket;
	int m_iAlivePlayer;
	int m_iSuccessNum;
	SGUser *m_pCurrentUser;
	SGUser *m_pEffectUser;

	int m_iGameState;

	SPRITE_CARD_INFO m_CARD[13];

	int m_iCardWidth;
	int m_iCardHeight;

	float m_fCardEffectTime;
	CARD_EFFECT_INFO m_CardEffectInfo;

	SCardInfo m_SelectedCard;

	SGStateManager* m_pStateManager;

	
};