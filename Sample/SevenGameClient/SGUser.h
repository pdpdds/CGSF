#pragma once

class SGManager;
class SGTable;

class SGUser  
{
public:
	SGUser();
	virtual ~SGUser();
	BOOL IsCloseCard(EdgeCard CardLimit,int iCardType);
	BOOL IsLowCardNum(int Low_Card,int iType);
	BOOL IsHighCardNum(int High_Card,int iType);
	
	int GetPassTicketNum(){return m_iPassTicketNum;};
	void DecreasePassTicketNum(){m_iPassTicketNum--;};
	void SetPassTicket(int iPassTicket){m_iPassTicketNum = iPassTicket;};

    SGManager *GetGameManager(){return m_pManager;};
	int GetID(){return m_iUserID;};
	void SetID(int iUserID){m_iUserID = iUserID;};
	void SetGameManager(SGManager *pManager){m_pManager = pManager; };
	void Initialize();
	void SetSevenCardNull();
	void EliminateCard(int iCardNum, int iCardType);	
	
	int GetRemainCard(){return m_TotalCardNum;};
	void SetRemainCard(int totalCard){m_TotalCardNum = totalCard;};
	int GetStatus(){return m_iStatus;};
	void SetStatus(int iStatus){m_iStatus=iStatus;};
	void SendUserCardToTable();

	int * GetSpadeArray(){return SpadeCardArray;};
	int * GetHeartArray(){return HeartCardArray;};
	int * GetDiamondArray(){return DiamondCardArray;};		
	int * GetCloverArray(){return CloverCardArray;};
	BOOL IHaveIt(int iCardNum, int iType);

	void AddCard(int cardType, int cardNum);

private:
	int m_iPassTicketNum;
	int m_iUserID;
	int m_iStatus;
	int m_TotalCardNum;
	SGManager *m_pManager;
	
	int SpadeCardArray[MAX_CARD_NUM+1];
	int HeartCardArray[MAX_CARD_NUM+1];
	int DiamondCardArray[MAX_CARD_NUM+1];
	int CloverCardArray[MAX_CARD_NUM+1];
	
};
