#pragma once
#include "Define.h"

class SGUser;
class SGTable;
class SGAlgorithm;

class SGManager  
{
public:
	SGManager();
	virtual ~SGManager();

	void AllocateObjcet(int iUserNum, int iPassTicket);
    void InitializeData();
	void CardDistribue();
	void FindSevenAndEliminate();
	void PlayerShuffle();
	void AddUser(int userSerial);
  
	void SendUserCardToTable(SGUser *pUser);		
	SGTable *GetTable(){return m_pTable;};
	SGUser * FindUser(int iUserID);

	void UpdateTableState(SCardInfo* pInfo);
	int GetUserNum(){return m_iUserNum;};
	
	
	BOOL CheckGameEnd();
	int GetNextUserID(int currentUserID);
	int EvaluateUser(SGUser *pUser);		
	SCardInfo ProcessAI(SGUser* pAI);
	BOOL ProcessTurnPass(int currentTurnUser);
	BOOL ProcessCardSubmit(int currentTurnUser, SCardInfo& info);


	int GetCurrentUserID(){return m_pCurrentUserID;};
	void SetCurrentUserID(int ID){m_pCurrentUserID = ID;}

	
	int m_iUserNum;
	int m_iCurrentTurn ;
	int m_iPassTicket;
	int m_iAlivePlayer;
	int m_iSuccessNum;
	int m_pCurrentUserID;

	int m_iGameState;
	BOOL m_bVictory;

	SGAlgorithm *m_pMainEngine;
    SGTable *m_pTable;

	std::vector<SGUser*>  m_userlist;
	std::list<int>  m_listUserOrder;
	std::vector<int>  m_vecWinner;
};