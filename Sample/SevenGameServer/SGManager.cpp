// SGManager.cpp: implementation of the SGManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SGManager.h"
#include "SGUser.h"
#include "SGTable.h"
#include "SelectAlgorithm.h"
#include <algorithm>
#include <functional>

SGManager::SGManager()
{
}

SGManager::~SGManager()
{
	Reset();

	if(m_pTable)
		delete m_pTable;

	if(m_pMainEngine)
		delete m_pMainEngine; 
}

void SGManager::Reset()
{
	for (auto& user : m_userlist)
	{
		delete user;
	}

	m_userlist.clear();
	m_listUserOrder.clear();
	m_vecWinner.clear();
}

void SGManager::AllocateObjcet(int iUserNum, int iPassTicket)
{
	m_pMainEngine = new SGAlgorithm();
    m_pTable = new SGTable();
	m_pMainEngine->SetTable(m_pTable);

	m_iPassTicket = iPassTicket;

	m_iUserNum = iUserNum;
}

void SGManager::AddUser(int userSerial)
{
	SGUser *pUser = new SGUser();
	pUser->SetID(userSerial);
	pUser->SetGameManager(this);
	m_userlist.push_back(pUser);
}

void SGManager::PlayerShuffle()
{
	m_listUserOrder.clear();
	m_vecWinner.clear();

	std::random_shuffle(m_userlist.begin(), m_userlist.end());

	for(auto& pUser : m_userlist)
	{
		m_listUserOrder.push_back(pUser->GetID());
	}
}

void SGManager::InitializeData()
{
	m_pTable->InitializeTable();
	m_iCurrentTurn = 0;
	m_iAlivePlayer = m_iUserNum;
	m_iSuccessNum = 1;

	srand( (unsigned)time( NULL ) );

	CardDistribue();
	FindSevenAndEliminate();
}



void SGManager::FindSevenAndEliminate()
{
	vector<SGUser*>::iterator pos  = m_userlist.begin();
	
	while ( pos != m_userlist.end()) 			
	{
		SGUser* pUser = (SGUser*)(*pos);

		pUser->SetSevenCardNull();
		pUser->SetPassTicket(m_iPassTicket);
		pUser->SetTotalCard();
		pUser->SetStatus(GAME_RUNNING);

		pos++;
	} 

	m_pTable->SetSevenCard();
}

void SGManager::CardDistribue()
{

	int paCardDistribute[MAX_CARD_NUM*4];
	
	memset(paCardDistribute,-1,MAX_CARD_NUM*4);
	
	int iCardCount = 0;
    while ( iCardCount < MAX_CARD_NUM*4 )
	{
		BOOL bRet = TRUE;
		int iCardNum  = rand() % (MAX_CARD_NUM*4);
		for(int i = 0 ; i< MAX_CARD_NUM*4 ; i++ )
		{
			if(paCardDistribute[i] == iCardNum)
			{
                bRet = FALSE;
				break;
			}
		}
		
		if(bRet == TRUE)
		{
			paCardDistribute[iCardCount] = iCardNum;
			iCardCount++;
		}
	}
    
	vector<SGUser*>::iterator pos  = m_userlist.begin();
	
	iCardCount= 0;
	int iUserIndex = 1;

	while ( pos != m_userlist.end()) 	 			
	{
		SGUser* pUser = (SGUser*)(*pos);

		pUser->Initialize();
		while ( iCardCount < iUserIndex * MAX_CARD_NUM )
		{
			pUser->UpdateUserCard(paCardDistribute[iCardCount]);
			iCardCount++;
		}
		iUserIndex++;

		pos++;		
	} 
}

SGUser * SGManager::FindUser(int iUserID)
{
	vector<SGUser*>::iterator pos  = m_userlist.begin();
	while ( pos != m_userlist.end())		
	{
		SGUser* pUser = (SGUser*)(*pos);
		
		if( iUserID == pUser->GetID() )
		{
			return pUser;				
		}	
		
		pos++;			
	} 
	return NULL;
}

int SGManager::GetNextUserID(int currentUserID)
{
	list<int>::iterator pos  = m_listUserOrder.begin();
	while ( pos != m_listUserOrder.end())
	{
		int id = (int)(*pos);		
		
		if(currentUserID == id )
		{												 				
			pos++;			
			
			if( pos == m_listUserOrder.end())		
			{
				pos  = m_listUserOrder.begin();
			}
			
			id = (int)(*pos);		
				
			return id;
		}					
		pos++;			
	} 

	return -10;
}

SCardInfo SGManager::ProcessAI(SGUser* pAI)
{
	SCardInfo selectCard = m_pMainEngine->GetNextAction(pAI);

	if( selectCard.iCardNum == -1 )
	{
		pAI->DecreasePassTicketCnt();	

	}
	else
	{
		pAI->EliminateCard(selectCard.iCardNum,selectCard.iCardType);	
	}	
		
	return selectCard;
}

int SGManager::EvaluateUser(SGUser *pUser)
{
	
	if( pUser->GetPassTicketCnt() < 0 )
	{
		pUser->SetStatus(GAME_END);
		SendUserCardToTable(pUser);
		pUser->SetRemainCardZero();
		m_iAlivePlayer--;		
		m_listUserOrder.remove(pUser->GetID());

		if(m_listUserOrder.size() == 1)
		{
			m_vecWinner.push_back(*m_listUserOrder.begin());
		}

		return -1;
	}
	else if( pUser->GetRemainCard() == 0 )
	{
		m_iSuccessNum++;
		m_iAlivePlayer--;
		pUser->SetStatus(GAME_END);
		pUser->SetRemainCardZero();
		m_listUserOrder.remove(pUser->GetID());
		m_vecWinner.push_back(pUser->GetID());

		return -2;
	}

	return 0;
}

void SGManager::SendUserCardToTable(SGUser *pUser)
{
	pUser->SendUserCardToTable();
}


BOOL SGManager::CheckGameEnd()
{
	
	if( m_iAlivePlayer <= 1)
	{			
		return TRUE;
	}	
	
	return FALSE;
}

void SGManager::UpdateTableState(SCardInfo* pInfo)
{
	m_pTable->UpdateTableState(pInfo->iCardNum,pInfo->iCardType);
}

BOOL SGManager::ProcessTurnPass(int currentTurnUser)
{
	SGUser* pUser = FindUser(currentTurnUser);

	if(pUser->GetPassTicketCnt() < 0)
		return FALSE;
	
	pUser->DecreasePassTicketCnt();

	return TRUE;
}
	
BOOL SGManager::ProcessCardSubmit(int currentTurnUser, SCardInfo& info)
{
	SGUser* pUser = FindUser(currentTurnUser);
	
	int iCardNum = info.iCardNum;
	int iCardType = info.iCardType;
		
	if( iCardNum >= 1 && iCardNum <= 13 && iCardType == TYPE_SPADE)
	{

		if(pUser->IHaveIt(iCardNum, TYPE_SPADE))
		{
			SGTable *pTable = GetTable();
			EdgeCard CardLimit = pTable->CheckBoundary(TYPE_SPADE);
			if(iCardNum != CardLimit.High_Card+1 && iCardNum != CardLimit.Low_Card-1 )
				return FALSE;

			pUser->EliminateCard(iCardNum,TYPE_SPADE);
			//pTable->SetSpadeCard(iCardNum,iCardNum);
			//	ResetField();
			return TRUE;
		}

	}

	//m_ctrHeartOut.GetWindowText(str);
	// iCardNum = atoi(str.GetBuffer(0));
	if( iCardNum >= 1 && iCardNum <= 13 && iCardType == TYPE_HEART)
	{

		if(pUser->IHaveIt(iCardNum, TYPE_HEART))
		{
			SGTable *pTable = GetTable();
			EdgeCard CardLimit = pTable->CheckBoundary(TYPE_HEART);
			if(iCardNum != CardLimit.High_Card+1 && iCardNum != CardLimit.Low_Card-1 )
				return FALSE;

			pUser->EliminateCard(iCardNum,TYPE_HEART);
			//pTable->SetHeartCard(iCardNum,iCardNum);
			//	ResetField();
			return TRUE;
		}

	}

	//m_ctrDiamondOut.GetWindowText(str);
	//	iCardNum = atoi(str.GetBuffer(0));
	if( iCardNum >= 1 && iCardNum <= 13 && iCardType == TYPE_DIAMOND)
	{

		if(pUser->IHaveIt(iCardNum, TYPE_DIAMOND))
		{

			SGTable *pTable = GetTable();
			EdgeCard CardLimit = pTable->CheckBoundary(TYPE_DIAMOND);
			if(iCardNum != CardLimit.High_Card+1 && iCardNum != CardLimit.Low_Card-1 )
				return FALSE;

			pUser->EliminateCard(iCardNum,TYPE_DIAMOND);
			//pTable->SetDiamondCard(iCardNum,iCardNum);
			//	ResetField();
			return TRUE;
		}	
	}

	//	m_ctrCloverOut.GetWindowText(str);
	//	iCardNum = atoi(str.GetBuffer(0));
	if( iCardNum >= 1 && iCardNum <= 13 && iCardType == TYPE_CLOVER)
	{
		if(pUser->IHaveIt(iCardNum, TYPE_CLOVER))
		{


			SGTable *pTable = GetTable();
			EdgeCard CardLimit = pTable->CheckBoundary(TYPE_CLOVER);
			if(iCardNum != CardLimit.High_Card+1 && iCardNum != CardLimit.Low_Card-1 )
				return FALSE;

			pUser->EliminateCard(iCardNum,TYPE_CLOVER);
			//pTable->SetCloverCard(iCardNum,iCardNum);
			//ResetField();
			return TRUE;
		}
	}

	return FALSE;
}
