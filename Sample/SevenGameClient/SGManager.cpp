// SGManager.cpp: implementation of the SGManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SGManager.h"
#include "SGUser.h"
#include "SGTable.h"
#include "SGStateManager.h"

SGManager::SGManager()
{
	m_fCardEffectTime = 0.0f;
	m_pStateManager = NULL;
}

SGManager::~SGManager()
{
	if(m_pTable)
		delete m_pTable;
	
	for(auto& user : m_userlist)
	{
		SGUser* pUser = user;
		delete pUser;
	}

	m_userlist.clear();
}

void SGManager::AllocateObjcet(int iUserNum, int iPassTicket)
{
	m_pStateManager = new SGStateManager(this);

	m_pStateManager->AddState(ENUM_SGINIT);
	m_pStateManager->AddState(ENUM_SGCARDEFFECT);
	m_pStateManager->AddState(ENUM_SGCARDTURN);
	m_pStateManager->AddState(ENUM_SGGAME);
	m_pStateManager->AddState(ENUM_SGSTART);
	m_pStateManager->AddState(ENUM_SGGAMEOVER);
	m_pStateManager->AddState(ENUM_SGCARDPASSEFFECT);

    m_pTable = new SGTable();
	
	m_iPassTicket = iPassTicket;
	m_iUserNum = iUserNum;
}


void SGManager::InitializeData()
{	
	m_pTable->InitializeTable();
	m_iCurrentTurn = -100;
	m_iAlivePlayer = m_iUserNum;
	m_iSuccessNum = 1;
	m_bIsVictory = FALSE;
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

void SGManager::AddUser(int userSerial)
{
	SGUser *pUser = new SGUser();
	pUser->SetID(userSerial);
	pUser->SetPassTicket(3);
	pUser->SetGameManager(this);
	m_userlist.push_back(pUser);
}

void SGManager::RemoveAllUser()
{
	for(auto& pUser : m_userlist)
	{
	   delete pUser;
	}

	m_userlist.clear();
}

int SGManager::GetNextUserID(int currentUserID)
{
	vector<int>::iterator pos  = m_vecActivePlayer.begin();
	while ( pos != m_vecActivePlayer.end())
	{
		int id = (int)(*pos);		
		
		if(currentUserID == id )
		{												 				
			pos = pos++;			
			
			if( pos == m_vecActivePlayer.end())		
			{
				pos  = m_vecActivePlayer.begin();
			}
			
			id = (int)(*pos);		
				
			return id;
		}					
		pos++;			
	} 

	return -10;
}

void SGManager::SendUserCardToTable(SGUser *pUser)
{
	pUser->SendUserCardToTable();
}

BOOL SGManager::MouseInPassButton(int iPosx,int iPosy) 
{


	if (iPosx > PASSBUTTON_LEFT && 
		iPosx < PASSBUTTON_RIGHT && 
		iPosy > PASSBUTTON_TOP && 
		iPosy < PASSBUTTON_BOTTOM )	
		return TRUE;

	return FALSE;
}

BOOL SGManager::CheckPassTicket(int iPosx,int iPosy) 
{
	if (iPosx > PASSBUTTON_LEFT && 
		iPosx < PASSBUTTON_RIGHT && 
		iPosy > PASSBUTTON_TOP && 
		iPosy < PASSBUTTON_BOTTOM )	
		return TRUE;

	return FALSE;
}

BOOL SGManager::UserProcessGame(int iPosX, int iPosY, SCardInfo& CardInfo)
{
	int iCardNum = -1;
	int iCardIndex = -1;
	for( int i=0; i<MAX_CARD_NUM; i++ )
	{

		if(m_CARD[i].CurrentRect.top<iPosY)
			if(m_CARD[i].CurrentRect.bottom>iPosY)
				if(m_CARD[i].CurrentRect.left<iPosX)
					if(m_CARD[i].CurrentRect.right>iPosX)
					{
						iCardNum = m_CARD[i].iCol+1;
						iCardIndex = i;		

						CardInfo.iCardNum = iCardNum;
						CardInfo.iCardType = m_CARD[iCardIndex].iCardType;

						return TRUE;
					}
	}

	return FALSE;
}

void SGManager::UpdateTableState(SCardInfo* pInfo)
{
	m_pTable->UpdateTableState(pInfo->iCardNum,pInfo->iCardType);
}

void SGManager::ReArrangeUserCard(SGUser* pUser,int &iCount,int iCardCnt)
{

	int *paArray;
	int i = 0;

	memset(&m_CARD,0,sizeof(SPRITE_CARD_INFO)*13);

	paArray = pUser->GetSpadeArray();
	for( i=1; i<=MAX_CARD_NUM; i++ )
	{

		if(paArray[i] == -1)
			continue;									
		m_CARD[iCount].iCardType = TYPE_SPADE;
		m_CARD[iCount].iRow = TYPE_SPADE;
		m_CARD[iCount].iCol = i-1;
		m_CARD[iCount].CurrentRect.top=(LONG)HUMAN_PLAYER_POS_Y;
		m_CARD[iCount].CurrentRect.left = (LONG)(HUMAN_PLAYER_POS_X + (iCount - ((iCardCnt) / 2)) * CARD_LOOKABLE_PART);
		m_CARD[iCount].CurrentRect.bottom = (LONG)(HUMAN_PLAYER_POS_Y + CARD_HEIGHT);
		m_CARD[iCount].CurrentRect.right = (LONG)(HUMAN_PLAYER_POS_X + (iCount - ((iCardCnt) / 2))*CARD_LOOKABLE_PART + CARD_LOOKABLE_PART);
		iCount++;
	}

	paArray = pUser->GetHeartArray();
	for( i=1; i<=MAX_CARD_NUM; i++ )
	{

		if(paArray[i] == -1)
			continue;									
		m_CARD[iCount].iCardType = TYPE_HEART;
		m_CARD[iCount].iRow = TYPE_HEART;
		m_CARD[iCount].iCol = i-1;
		m_CARD[iCount].CurrentRect.top = (LONG)HUMAN_PLAYER_POS_Y;
		m_CARD[iCount].CurrentRect.left = (LONG)(HUMAN_PLAYER_POS_X + (iCount - ((iCardCnt) / 2)) * CARD_LOOKABLE_PART);
		m_CARD[iCount].CurrentRect.bottom = (LONG)(HUMAN_PLAYER_POS_Y + CARD_HEIGHT);
		m_CARD[iCount].CurrentRect.right = (LONG)(HUMAN_PLAYER_POS_X + (iCount - ((iCardCnt) / 2))*CARD_LOOKABLE_PART + CARD_LOOKABLE_PART);
		iCount++;
	}

	paArray = pUser->GetDiamondArray();
	for( i=1; i<=MAX_CARD_NUM; i++ )
	{

		if(paArray[i] == -1)
			continue;									
		m_CARD[iCount].iCardType = TYPE_DIAMOND;
		m_CARD[iCount].iRow = TYPE_DIAMOND;
		m_CARD[iCount].iCol = i-1;
		m_CARD[iCount].CurrentRect.top = (LONG)HUMAN_PLAYER_POS_Y;
		m_CARD[iCount].CurrentRect.left = (LONG)(HUMAN_PLAYER_POS_X + (iCount - ((iCardCnt) / 2)) * CARD_LOOKABLE_PART);
		m_CARD[iCount].CurrentRect.bottom = (LONG)(HUMAN_PLAYER_POS_Y + CARD_HEIGHT);
		m_CARD[iCount].CurrentRect.right = (LONG)(HUMAN_PLAYER_POS_X + (iCount - ((iCardCnt) / 2))*CARD_LOOKABLE_PART + CARD_LOOKABLE_PART);
		iCount++;
	}

	paArray = pUser->GetCloverArray();
	for( i=1; i<=MAX_CARD_NUM; i++ )
	{

		if(paArray[i] == -1)
			continue;									
		m_CARD[iCount].iCardType = TYPE_CLOVER;
		m_CARD[iCount].iRow = TYPE_CLOVER;
		m_CARD[iCount].iCol = i-1;
		m_CARD[iCount].CurrentRect.top = (LONG)HUMAN_PLAYER_POS_Y;
		m_CARD[iCount].CurrentRect.left = (LONG)(HUMAN_PLAYER_POS_X + (iCount - ((iCardCnt) / 2)) * CARD_LOOKABLE_PART);
		m_CARD[iCount].CurrentRect.bottom = (LONG)(HUMAN_PLAYER_POS_Y + CARD_HEIGHT);
		m_CARD[iCount].CurrentRect.right = (LONG)(HUMAN_PLAYER_POS_X + (iCount - ((iCardCnt) / 2))*CARD_LOOKABLE_PART + CARD_LOOKABLE_PART);
		iCount++;		
	}

	if(iCount >= 1)
		m_CARD[iCount-1].CurrentRect.right=(LONG)(HUMAN_PLAYER_POS_X+((iCount-1) -((iCardCnt)/2))*CARD_LOOKABLE_PART + CARD_WIDTH);				
}

BOOL SGManager::GetCardImageIndex( SCardInfo* pInfo, int& iCol, int& iRow )
{
	switch(pInfo->iCardType)
	{		
	case TYPE_CLOVER:
		iRow = 0;
		break;
	case TYPE_HEART:
		iRow = 1;
		break;
	case TYPE_DIAMOND:
		iRow = 2;
		break;
	case TYPE_SPADE:
		iRow = 3;
		break;
	}

	iCol = pInfo->iCardNum - 1;
	
	return TRUE;
}

void SGManager::SetCardEffectInfo( SCardInfo* pInfo )
{
	SGUser* pUser = GetEffectUser();

	int CardNum = pUser->GetRemainCard();

	int count = 0;

	for(auto& user : m_vecDisplayOrder)
	{
		if(user == pUser->GetID())
			break;

		count++;
	}

	switch(count)
	{
	case 0:
		{
			m_CardEffectInfo.iOriX = SILENCE_PLAYER_CARD_POS_X;
			m_CardEffectInfo.iOriY = SILENCE_PLAYER_CARD_POS_Y;
		}
		break;
	case 1:
		{
			m_CardEffectInfo.iOriX = DETECTIVE_PLAYER_POS_X;
			m_CardEffectInfo.iOriY = DETECTIVE_PLAYER_POS_Y;
		}
		break;
	case 2:
		{
			m_CardEffectInfo.iOriX = HUMAN_PLAYER_POS_X;
			m_CardEffectInfo.iOriY = HUMAN_PLAYER_POS_Y;
		}
		break;
	case 3:
		{
			m_CardEffectInfo.iOriX = GENIUS_PLAYER_POS_X;
			m_CardEffectInfo.iOriY = GENIUS_PLAYER_POS_Y;
		}
		break;
	}

	switch(pInfo->iCardType)
	{		
	case TYPE_CLOVER:
		m_CardEffectInfo.iDestX = (int)((pInfo->iCardNum - 1)*CARD_LOOKABLE_PART + DEFAULT_TABLE_CARD_POS_X);
		m_CardEffectInfo.iDestY = (int)(DEFAULT_TABLE_CARD_POS_Y + CARD_DISTANCE * 3);
		break;
	case TYPE_HEART:
		m_CardEffectInfo.iDestX = (int)((pInfo->iCardNum - 1)*CARD_LOOKABLE_PART + DEFAULT_TABLE_CARD_POS_X);
		m_CardEffectInfo.iDestY = (int)(DEFAULT_TABLE_CARD_POS_Y + CARD_DISTANCE * 1);
		break;
	case TYPE_DIAMOND:
		m_CardEffectInfo.iDestX = (int)((pInfo->iCardNum - 1)*CARD_LOOKABLE_PART + DEFAULT_TABLE_CARD_POS_X);
		m_CardEffectInfo.iDestY = (int)(DEFAULT_TABLE_CARD_POS_Y + CARD_DISTANCE * 2);
		break;
	case TYPE_SPADE:
		m_CardEffectInfo.iDestX = (int)((pInfo->iCardNum - 1)*CARD_LOOKABLE_PART + DEFAULT_TABLE_CARD_POS_X);
		m_CardEffectInfo.iDestY = (int)DEFAULT_TABLE_CARD_POS_Y;
		break;
	}
}

BOOL SGManager::OnRender( float fElapsedTime )
{
	if(m_pStateManager)
		return m_pStateManager->OnRender(fElapsedTime);

	return FALSE;
}

BOOL SGManager::OnMessage( int iX, int iY )
{
	if(m_pStateManager)
		return m_pStateManager->OnMessage(iX, iY);

	return FALSE;
}

BOOL SGManager::ChangeState( eSGState state )
{
	if(m_pStateManager)
		m_pStateManager->ChangeState(state);

	return FALSE;
}