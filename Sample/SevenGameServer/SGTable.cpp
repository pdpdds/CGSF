#include "stdafx.h"
#include "SGTable.h"

SGTable::SGTable()
{

}

SGTable::~SGTable()
{

}

void SGTable::InitializeTable()
{
    
	for( int i=0 ; i<MAX_CARD_NUM +1 ; i++ )
	{
		SpadeCardArray[i] = -1;
		HeartCardArray[i] = -1;
		DiamondCardArray[i] = -1;
		CloverCardArray[i] = -1;
	}
}

void SGTable::SetSevenCard()
{
	SpadeCardArray[7] = 7;
	HeartCardArray[7] = 7; 
	DiamondCardArray[7] = 7;
	CloverCardArray[7] = 7;
}
	
void SGTable::UpdateTableState(int iCardNum, int iCardType)
{

	if(iCardType == TYPE_SPADE)
	{
		SpadeCardArray[iCardNum] = iCardNum;
	}
	if(iCardType == TYPE_HEART)
	{
		HeartCardArray[iCardNum] = iCardNum;
	}
	if(iCardType == TYPE_CLOVER)
	{
		CloverCardArray[iCardNum] = iCardNum;
	}
	if(iCardType == TYPE_DIAMOND)
	{
		DiamondCardArray[iCardNum] = iCardNum;
	}
}


EdgeCard SGTable::CheckBoundary(int iCardType)
{
	EdgeCard CardBound;
	CardBound.High_Card = 13;
	CardBound.Low_Card = 1;

	int i = 0;
	if(iCardType == TYPE_SPADE)
	{
		for(i = 6; i>= 1; i--)
		{
			if( SpadeCardArray[i]  == -1 )
			{
				CardBound.Low_Card = i+1;
				break;
			}
		}

		for(i = 8; i <= MAX_CARD_NUM ; i++)
		{
			if( SpadeCardArray[i]  == -1 )
			{
				CardBound.High_Card = i-1;
				break;
			}
		}
	}

	if(iCardType == TYPE_HEART)
	{
		for(i = 6; i>= 1; i--)
		{
			if( HeartCardArray[i]  == -1 )
			{
				CardBound.Low_Card = i+1;
				break;
			}
		}

		for(i = 8; i <= MAX_CARD_NUM ; i++)
		{
			if( HeartCardArray[i]  == -1 )
			{
				CardBound.High_Card = i-1;
				break;
			}
		}
	}

	if(iCardType == TYPE_DIAMOND)
	{
		for(i = 6; i>= 1; i--)
		{
			if( DiamondCardArray[i]  == -1 )
			{
				CardBound.Low_Card = i+1;
				break;
			}
		}

		for(i = 8; i <= MAX_CARD_NUM ; i++)
		{
			if( DiamondCardArray[i]  == -1 )
			{
				CardBound.High_Card = i-1;
				break;
			}
		}
	}

	if(iCardType == TYPE_CLOVER)
	{
		for(i = 6; i>= 1; i--)
		{
			if( CloverCardArray[i]  == -1 )
			{
				CardBound.Low_Card = i+1;
				break;
			}
		}

		for(i = 8; i <= MAX_CARD_NUM ; i++)
		{
			if( CloverCardArray[i]  == -1 )
			{
				CardBound.High_Card = i-1;
				break;
			}
		}
	}

	return CardBound;
}
