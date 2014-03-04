#include "stdafx.h"
#include "SGUser.h"
#include "SGTable.h"
#include "SGManager.h"

SGUser::SGUser()
{

}

SGUser::~SGUser()
{

}

void SGUser::Initialize()
{
    
	for( int i=0 ; i<MAX_CARD_NUM +1 ; i++ )
	{
		SpadeCardArray[i] = -1;
		HeartCardArray[i] = -1;
		DiamondCardArray[i] = -1;
		CloverCardArray[i] = -1;
	}
}


void SGUser::AddCard(int cardType, int cardNum)
{
	if( cardType == TYPE_SPADE)
		SpadeCardArray[cardNum] = cardNum;

	if( cardType == TYPE_HEART)
		HeartCardArray[cardNum] = cardNum;

	if( cardType == TYPE_DIAMOND)
		DiamondCardArray[cardNum] = cardNum;

	if( cardType == TYPE_CLOVER)
		CloverCardArray[cardNum] = cardNum;
}

void SGUser::SetSevenCardNull()
{
	SpadeCardArray[7] = -1;
	HeartCardArray[7] = -1; 
	DiamondCardArray[7] = -1;
	CloverCardArray[7] = -1;
}


BOOL SGUser::IHaveIt(int iCardNum, int iCardType)
{	
	if(iCardType == TYPE_SPADE)
	{
		if( SpadeCardArray[iCardNum] != -1 )
			return TRUE;
	}

	if(iCardType == TYPE_HEART)
	{
		if( HeartCardArray[iCardNum] != -1 )
			return TRUE;
	}

	if(iCardType == TYPE_DIAMOND)
	{
		if( DiamondCardArray[iCardNum] != -1 )
			return TRUE;
	}
	
	if(iCardType == TYPE_CLOVER)
	{
		if( CloverCardArray[iCardNum] != -1 )
			return TRUE;
	}

	return FALSE;
}

BOOL SGUser::IsCloseCard(EdgeCard CardLimit,int iCardType)
{
	if(iCardType == TYPE_SPADE)
	{
		if(CardLimit.Low_Card > 1 && CardLimit.Low_Card <= 7)
		{
			if( SpadeCardArray[CardLimit.Low_Card - 1] != -1 )
				return TRUE;
		}
		if(CardLimit.High_Card >= 7 && CardLimit.High_Card < 13)
		{
			if( SpadeCardArray[CardLimit.High_Card + 1] != -1 )
				return TRUE;
		}
		
	}

	if(iCardType == TYPE_HEART)
	{
		if(CardLimit.Low_Card > 1 && CardLimit.Low_Card <= 7)
		{
			if( HeartCardArray[CardLimit.Low_Card - 1] != -1 )
				return TRUE;
		}
		if(CardLimit.High_Card >= 7 && CardLimit.High_Card < 13)
		{
			if( HeartCardArray[CardLimit.High_Card + 1] != -1 )
				return TRUE;
		}
		
	}

	if(iCardType == TYPE_DIAMOND)
	{
		if(CardLimit.Low_Card > 1 && CardLimit.Low_Card <= 7)
		{
			if( DiamondCardArray[CardLimit.Low_Card - 1] != -1 )
				return TRUE;
		}
		if(CardLimit.High_Card >= 7 && CardLimit.High_Card < 13)
		{
			if( DiamondCardArray[CardLimit.High_Card + 1] != -1 )
				return TRUE;
		}
		
	}
	
	if(iCardType == TYPE_CLOVER)
	{
		if(CardLimit.Low_Card > 1 && CardLimit.Low_Card <= 7)
		{
			if( CloverCardArray[CardLimit.Low_Card - 1] != -1 )
				return TRUE;
		}
		if(CardLimit.High_Card >= 7 && CardLimit.High_Card < 13)
		{
			if( CloverCardArray[CardLimit.High_Card + 1] != -1 )
				return TRUE;
		}
		
	}
	return FALSE;
}

int SGUser::IsLowCardNum(int Low_Card,int iCardType)
{	
	if(iCardType == TYPE_CLOVER)
	{
		if( Low_Card == 1 )
			return -1;
		else
		{
			if( CloverCardArray[Low_Card - 1] != -1 )
			{
				return CloverCardArray[Low_Card - 1];
			}
		}
	}
	if(iCardType == TYPE_DIAMOND)
	{
		if( Low_Card == 1 )
			return -1;
		else
		{
			if( DiamondCardArray[Low_Card - 1] != -1 )
			{
				return DiamondCardArray[Low_Card - 1];
			}
		}
	}
	
	if(iCardType == TYPE_HEART)
	{
		if( Low_Card == 1 )
			return -1;
		else
		{
			if( HeartCardArray[Low_Card - 1] != -1 )
			{
				return HeartCardArray[Low_Card - 1];
			}
		}
	}
			
		if(iCardType == TYPE_SPADE)
	{
		if( Low_Card == 1 )
			return -1;
		else
		{
			if( SpadeCardArray[Low_Card - 1] != -1 )
			{
				return SpadeCardArray[Low_Card - 1];
			}
		}
	}
	return -1;
}
BOOL SGUser::IsHighCardNum(int High_Card,int iCardType)
{	
	if(iCardType == TYPE_CLOVER)
	{
		if( High_Card == MAX_CARD_NUM )
			return -1;
		else
		{
			if( CloverCardArray[High_Card + 1] != -1 )
			{
				return CloverCardArray[High_Card + 1];
			}
			else
			{
			}
		}
	}
	if(iCardType == TYPE_DIAMOND)
	{
		if( High_Card == MAX_CARD_NUM )
			return -1;
		else
		{
			if( DiamondCardArray[High_Card + 1] != -1 )
			{
				return DiamondCardArray[High_Card + 1];
			}
		}
	}
	
	if(iCardType == TYPE_HEART)
	{
		if( High_Card == MAX_CARD_NUM )
			return -1;
		else
		{
			if( HeartCardArray[High_Card + 1] != -1 )
			{
				return HeartCardArray[High_Card + 1];
			}
		}
	}
			
	if(iCardType == TYPE_SPADE)
	{
		if( High_Card == MAX_CARD_NUM )
			return -1;
		else
		{
			if( SpadeCardArray[High_Card + 1] != -1 )
			{
				return SpadeCardArray[High_Card + 1];
			}
		}
	}
	return -1;
}


void SGUser::EliminateCard(int iCardNum, int iCardType)
{

	if(iCardType == TYPE_SPADE)
	{
		SpadeCardArray[iCardNum] = -1;
		m_TotalCardNum--;
	}
	if(iCardType == TYPE_HEART)
	{
		HeartCardArray[iCardNum] = -1;
		m_TotalCardNum--;
	}
	if(iCardType == TYPE_CLOVER)
	{
		CloverCardArray[iCardNum] = -1;
		m_TotalCardNum--;
	}
	if(iCardType == TYPE_DIAMOND)
	{
		DiamondCardArray[iCardNum] = -1;
		m_TotalCardNum--;
	}
}



void SGUser::SendUserCardToTable()
{
	SGManager *pManager = GetGameManager();
	SGTable *pTable = pManager->GetTable();

	for( int i=1 ; i<= MAX_CARD_NUM ; i++)
	{
		if( SpadeCardArray[i] != -1)
			pTable->SetSpadeCard(SpadeCardArray[i],i);

		if( HeartCardArray[i] != -1)	
			pTable->SetHeartCard(HeartCardArray[i],i);

		if( DiamondCardArray[i] != -1)
			pTable->SetDiamondCard(DiamondCardArray[i],i);

		if( CloverCardArray[i] != -1)
			pTable->SetCloverCard(CloverCardArray[i],i);
	}
}