#pragma once

class SGTable;
class SGUser;
class SGManager;

class SGAlgorithm  
{
public:
	SGAlgorithm();
	virtual ~SGAlgorithm();
	SGManager *GetGameManager();
	BOOL IMustPass(int iCountGrapedCard,int iCountCoocockCard,int iCountGetOutCard,SGUser *pUser);
	SGTable *GetTableInstance(){return m_pTable;};
	void SetTable(SGTable *pTable){m_pTable = pTable;};
	SCardInfo GetNextAction(SGUser *pUser);

private:
	SGTable *m_pTable;
	void SetTableInstance(SGTable *pTable) {m_pTable = pTable;}
	SGUser *GetUserData();
	void SetUserData();
	
	int CheckPassCard(SGUser *pUser);
		
	BOOL CheckOtherUserWithProb(SGUser *pUser, int iProb);
	int FindCardPriorty(int iCardNum,int iType,SGUser *pUser);
	SCardInfo SelectCard(SGUser *pUser);
	BOOL CheckProperCard(SGTable *pTable, SGUser *pUser);
	BOOL DeterminePassCard(SGTable *pTable, SGUser *pUser);

};