#include "stdafx.h"
#include "SelectAlgorithm.h"
#include "SGUser.h"
#include "SGTable.h"
#include "SGManager.h"

SGAlgorithm::SGAlgorithm()
{

}

SGAlgorithm::~SGAlgorithm()
{
	

}
SCardInfo SGAlgorithm::GetNextAction(SGUser *pUser)
{
	int iPass = CheckPassCard(pUser);
	if( iPass == CARD_PASS_OK ) 
	{
		SCardInfo sCardPass;
		sCardPass.iCardNum = -1;
		sCardPass.iCardType = -1;
		return sCardPass;
	}	
	return SelectCard(pUser);
}

SCardInfo SGAlgorithm::SelectCard(SGUser *pUser)
{
	SGTable *pTable = GetTableInstance();
	SCardInfo SSelectCard;
	int iSelectCardPriorty = PRIORTY_A;
	EdgeCard CardLimit;
	CardLimit = pTable->CheckBoundary(TYPE_SPADE);
	if( pUser->IsCloseCard(CardLimit,TYPE_SPADE) == TRUE )
	{
		int iCardNum = -1;
		iCardNum = pUser->IsLowCardNum(CardLimit.Low_Card,TYPE_SPADE);
		if( iCardNum != -1 )
		{
			int iCurrentCardPriorty = PRIORTY_A;
			iCurrentCardPriorty = FindCardPriorty(iCardNum,TYPE_SPADE,pUser);
			if(iCurrentCardPriorty >= iSelectCardPriorty)
			{
				iSelectCardPriorty = iCurrentCardPriorty;
				SSelectCard.iCardNum = iCardNum;
				SSelectCard.iCardType = TYPE_SPADE;
			}
		}

		iCardNum = -1;
		iCardNum = pUser->IsHighCardNum(CardLimit.High_Card,TYPE_SPADE);
		if( iCardNum != -1 )
		{
			int iCurrentCardPriorty = PRIORTY_A;
			iCurrentCardPriorty = FindCardPriorty(iCardNum,TYPE_SPADE,pUser);
			if(iCurrentCardPriorty >= iSelectCardPriorty)
			{
				iSelectCardPriorty = iCurrentCardPriorty;
				SSelectCard.iCardNum = iCardNum;
				SSelectCard.iCardType = TYPE_SPADE;
			}	
		}
	}

	CardLimit = pTable->CheckBoundary(TYPE_HEART);
	if( pUser->IsCloseCard(CardLimit,TYPE_HEART) == TRUE )
	{
		int iCardNum = -1;
		iCardNum = pUser->IsLowCardNum(CardLimit.Low_Card,TYPE_HEART);
		if( iCardNum != -1 )
		{
			int iCurrentCardPriorty = PRIORTY_A;
			iCurrentCardPriorty = FindCardPriorty(iCardNum,TYPE_HEART,pUser);
			if(iCurrentCardPriorty >= iSelectCardPriorty)
			{
				iSelectCardPriorty = iCurrentCardPriorty;
				SSelectCard.iCardNum = iCardNum;
				SSelectCard.iCardType = TYPE_HEART;
			}
		}

		iCardNum = -1;
		iCardNum = pUser->IsHighCardNum(CardLimit.High_Card,TYPE_HEART);
		if( iCardNum != -1 )
		{
			int iCurrentCardPriorty = PRIORTY_A;
			iCurrentCardPriorty = FindCardPriorty(iCardNum,TYPE_HEART,pUser);
			if(iCurrentCardPriorty >= iSelectCardPriorty)
			{
				iSelectCardPriorty = iCurrentCardPriorty;
				SSelectCard.iCardNum = iCardNum;
				SSelectCard.iCardType = TYPE_HEART;
			}	
		}
	}

	CardLimit = pTable->CheckBoundary(TYPE_CLOVER);
	if( pUser->IsCloseCard(CardLimit,TYPE_CLOVER) == TRUE )
	{
		int iCardNum = -1;
		iCardNum = pUser->IsLowCardNum(CardLimit.Low_Card,TYPE_CLOVER);
		if( iCardNum != -1 )
		{
			int iCurrentCardPriorty = PRIORTY_A;
			iCurrentCardPriorty = FindCardPriorty(iCardNum,TYPE_CLOVER,pUser);
			if(iCurrentCardPriorty >= iSelectCardPriorty)
			{
				iSelectCardPriorty = iCurrentCardPriorty;
				SSelectCard.iCardNum = iCardNum;
				SSelectCard.iCardType = TYPE_CLOVER;
			}
		}

		iCardNum = -1;
		iCardNum = pUser->IsHighCardNum(CardLimit.High_Card,TYPE_CLOVER);
		if( iCardNum != -1 )
		{
			int iCurrentCardPriorty = PRIORTY_A;
			iCurrentCardPriorty = FindCardPriorty(iCardNum,TYPE_CLOVER,pUser);
			if(iCurrentCardPriorty >= iSelectCardPriorty)
			{
				iSelectCardPriorty = iCurrentCardPriorty;
				SSelectCard.iCardNum = iCardNum;
				SSelectCard.iCardType = TYPE_CLOVER;
			}	
		}
	}

	CardLimit = pTable->CheckBoundary(TYPE_DIAMOND);
	if( pUser->IsCloseCard(CardLimit,TYPE_DIAMOND) == TRUE )
	{
		int iCardNum = -1;
		iCardNum = pUser->IsLowCardNum(CardLimit.Low_Card,TYPE_DIAMOND);
		if( iCardNum != -1 )
		{
			int iCurrentCardPriorty = PRIORTY_A;
			iCurrentCardPriorty = FindCardPriorty(iCardNum,TYPE_DIAMOND,pUser);
			if(iCurrentCardPriorty >= iSelectCardPriorty)
			{
				iSelectCardPriorty = iCurrentCardPriorty;
				SSelectCard.iCardNum = iCardNum;
				SSelectCard.iCardType = TYPE_DIAMOND;
			}
		}

		iCardNum = -1;
		iCardNum = pUser->IsHighCardNum(CardLimit.High_Card,TYPE_DIAMOND);
		if( iCardNum != -1 )
		{
			int iCurrentCardPriorty = PRIORTY_A;
			iCurrentCardPriorty = FindCardPriorty(iCardNum,TYPE_DIAMOND,pUser);
			if(iCurrentCardPriorty >= iSelectCardPriorty)
			{
				iSelectCardPriorty = iCurrentCardPriorty;
				SSelectCard.iCardNum = iCardNum;
				SSelectCard.iCardType = TYPE_DIAMOND;
			}	
		}
	}


	return SSelectCard;
}

// this routine determines whether user should put card on table
int SGAlgorithm::CheckPassCard(SGUser *pUser)
{
	SGTable *pTable = GetTableInstance();
	if( pTable == NULL)
		return ERROR_TABLE_NULL;

	if( pUser == NULL)
		return ERROR_USER_NULL;

	if( CheckProperCard(pTable, pUser) == FALSE )
		return CARD_PASS_OK;

	if( DeterminePassCard(pTable, pUser) == TRUE ) //very important routine
		return CARD_PASS_OK;

	return CARD_PASS_NO;
    
}

int SGAlgorithm::FindCardPriorty(int iCardNum,int iCardType,SGUser *pUser)
{
	int iCheckCard = -1;
	int CountCard = 0;
	
	if( iCardNum == 1 )
	{
		return PRIORTY_F;	
	}

	if( iCardNum == 13 )
	{
		return PRIORTY_F;	
	}
	
	if( iCardNum == 2 )
	{
		iCheckCard = pUser->IsLowCardNum(iCardNum,iCardType);
		if( iCheckCard != -1)
		{
			return PRIORTY_F;
		}	
		else return PRIORTY_D;
	}

	if( iCardNum == 12 )
	{
		iCheckCard = pUser->IsHighCardNum(iCardNum,iCardType);
		if( iCheckCard != -1)
		{
			return PRIORTY_F;
		}	
		else return PRIORTY_D;
	}

	if( iCardNum == 3 )
	{
		iCheckCard = pUser->IsLowCardNum(iCardNum,iCardType);
		if( iCheckCard != -1)
		{
			return PRIORTY_F;
		}	
		else
		{
			iCheckCard = pUser->IsLowCardNum(iCardNum-1,iCardType);	
			if( iCheckCard != -1 )
			{
				return PRIORTY_E;
			}
			else return PRIORTY_D;
		}

	}

	if( iCardNum == 11 )
	{
		iCheckCard = pUser->IsHighCardNum(iCardNum,iCardType);
		if( iCheckCard != -1)
		{
			return PRIORTY_F;
		}	
		else
		{
			iCheckCard = pUser->IsHighCardNum(iCardNum+1,iCardType);	
			if( iCheckCard != -1 )
			{
				return PRIORTY_E;
			}
			else return PRIORTY_D;
		}

	}

	if( iCardNum == 4 )
	{
		iCheckCard = pUser->IsLowCardNum(iCardNum,iCardType);
		if( iCheckCard != -1)
		{
			return PRIORTY_F;
		}	
		else
		{
			
			iCheckCard = pUser->IsLowCardNum(iCardNum-1,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}
			iCheckCard = pUser->IsLowCardNum(iCardNum-2,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}
			if ( CountCard > 0 )
			{
				return PRIORTY_E;
			}
			else return PRIORTY_C;		
		}
	}

		
	if( iCardNum == 10 )
	{
		iCheckCard = pUser->IsHighCardNum(iCardNum,iCardType);
		if( iCheckCard != -1)
		{
			return PRIORTY_F;
		}	
		else
		{
			
			iCheckCard = pUser->IsHighCardNum(iCardNum+1,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}
			iCheckCard = pUser->IsHighCardNum(iCardNum+2,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}
			if ( CountCard > 0 )
			{
				return PRIORTY_E;
			}
			else return PRIORTY_C;		
		}
	}



	if( iCardNum == 5 )
	{
		iCheckCard = pUser->IsLowCardNum(iCardNum,iCardType);
		if( iCheckCard != -1)
		{
			return PRIORTY_F;
		}	
		else
		{
			
			iCheckCard = pUser->IsLowCardNum(iCardNum-1,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}
			iCheckCard = pUser->IsLowCardNum(iCardNum-2,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}
			iCheckCard = pUser->IsLowCardNum(iCardNum-3,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}

			if ( CountCard > 0 )
			{
				return PRIORTY_E;
			}
			else return PRIORTY_B;		
		}
	}

	if( iCardNum == 9 )
	{
		iCheckCard = pUser->IsHighCardNum(iCardNum,iCardType);
		if( iCheckCard != -1)
		{
			return PRIORTY_F;
		}	
		else
		{
			
			iCheckCard = pUser->IsHighCardNum(iCardNum+1,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}
			iCheckCard = pUser->IsHighCardNum(iCardNum+2,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}
			iCheckCard = pUser->IsHighCardNum(iCardNum+3,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}

			if ( CountCard > 0 )
			{
				return PRIORTY_E;
			}
			else return PRIORTY_B;		
		}
	}
	
	if( iCardNum == 6 )
	{
		iCheckCard = pUser->IsLowCardNum(iCardNum,iCardType);
		if( iCheckCard != -1)
		{
			return PRIORTY_F;
		}	
		else
		{
			
			iCheckCard = pUser->IsLowCardNum(iCardNum-1,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}
			iCheckCard = pUser->IsLowCardNum(iCardNum-2,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}
			iCheckCard = pUser->IsLowCardNum(iCardNum-3,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}

			iCheckCard = pUser->IsLowCardNum(iCardNum-4,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}

			if ( CountCard > 0 )
			{
				return PRIORTY_E;
			}
			else return PRIORTY_A;		
		}
	}
	
	if( iCardNum == 8 )
	{
		iCheckCard = pUser->IsHighCardNum(iCardNum,iCardType);
		if( iCheckCard != -1)
		{
			return PRIORTY_F;
		}	
		else
		{
			
			iCheckCard = pUser->IsHighCardNum(iCardNum+1,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}
			iCheckCard = pUser->IsHighCardNum(iCardNum+2,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}
			iCheckCard = pUser->IsHighCardNum(iCardNum+3,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}

			iCheckCard = pUser->IsHighCardNum(iCardNum+4,iCardType);	
			if( iCheckCard != -1 )
			{
				CountCard ++;
			}

			if ( CountCard > 0 )
			{
				return PRIORTY_E;
			}
			else return PRIORTY_A;		
		}
	}

	
	return -1;
	
}

BOOL SGAlgorithm::DeterminePassCard(SGTable *pTable, SGUser *pUser)
{
	EdgeCard CardLimit;
	int iCountGrapedCard = 0;
	int iCountCoocockCard = 0;
	int iCountGetOutCard = 0;

	CardLimit = pTable->CheckBoundary(TYPE_SPADE);
	if( pUser->IsCloseCard(CardLimit,TYPE_SPADE) == TRUE )
	{
		int iCardNum = -1;
		iCardNum = pUser->IsLowCardNum(CardLimit.Low_Card,TYPE_SPADE);
		if( iCardNum != -1 )
		{
			int iPriorty;
			iPriorty = FindCardPriorty(iCardNum,TYPE_SPADE,pUser);
			if( iPriorty == PRIORTY_A || iPriorty == PRIORTY_B || iPriorty == PRIORTY_C || iPriorty ==  PRIORTY_D )
				iCountGrapedCard++;
			else if( iPriorty == PRIORTY_E )
				iCountCoocockCard++;
			else if( iPriorty == PRIORTY_F )
				iCountGetOutCard++;
		}

		iCardNum = -1;
		iCardNum = pUser->IsHighCardNum(CardLimit.High_Card,TYPE_SPADE);
		if( iCardNum != -1 )
		{
			int iPriorty;
			iPriorty = FindCardPriorty(iCardNum,TYPE_SPADE,pUser);
			if( iPriorty == PRIORTY_A || iPriorty == PRIORTY_B || iPriorty == PRIORTY_C || iPriorty ==  PRIORTY_D )
				iCountGrapedCard++;
			else if( iPriorty == PRIORTY_E )
				iCountCoocockCard++;
			else if( iPriorty == PRIORTY_F )
				iCountGetOutCard++;
		}

	}

	CardLimit = pTable->CheckBoundary(TYPE_DIAMOND);
	if( pUser->IsCloseCard(CardLimit,TYPE_DIAMOND) == TRUE )
	{
		int iCardNum = -1;
		iCardNum = pUser->IsLowCardNum(CardLimit.Low_Card,TYPE_DIAMOND);
		if( iCardNum != -1 )
		{
			int iPriorty;
			iPriorty = FindCardPriorty(iCardNum,TYPE_DIAMOND,pUser);
			if( iPriorty == PRIORTY_A || iPriorty == PRIORTY_B || iPriorty == PRIORTY_C || iPriorty ==  PRIORTY_D )
				iCountGrapedCard++;
			else if( iPriorty == PRIORTY_E )
				iCountCoocockCard++;
			else if( iPriorty == PRIORTY_F )
				iCountGetOutCard++;
		}

		iCardNum = -1;
		iCardNum = pUser->IsHighCardNum(CardLimit.High_Card,TYPE_DIAMOND);
		if( iCardNum != -1 )
		{
			int iPriorty;
			iPriorty = FindCardPriorty(iCardNum,TYPE_DIAMOND,pUser);
			if( iPriorty == PRIORTY_A || iPriorty == PRIORTY_B || iPriorty == PRIORTY_C || iPriorty ==  PRIORTY_D )
				iCountGrapedCard++;
			else if( iPriorty == PRIORTY_E )
				iCountCoocockCard++;
			else if( iPriorty == PRIORTY_F )
				iCountGetOutCard++;
		}

	}

	CardLimit = pTable->CheckBoundary(TYPE_CLOVER);
	if( pUser->IsCloseCard(CardLimit,TYPE_CLOVER) == TRUE )
	{
		int iCardNum = -1;
		iCardNum = pUser->IsLowCardNum(CardLimit.Low_Card,TYPE_CLOVER);
		if( iCardNum != -1 )
		{
			int iPriorty;
			iPriorty = FindCardPriorty(iCardNum,TYPE_CLOVER,pUser);
			if( iPriorty == PRIORTY_A || iPriorty == PRIORTY_B || iPriorty == PRIORTY_C || iPriorty ==  PRIORTY_D )
				iCountGrapedCard++;
			else if( iPriorty == PRIORTY_E )
				iCountCoocockCard++;
			else if( iPriorty == PRIORTY_F )
				iCountGetOutCard++;
		}

		iCardNum = -1;
		iCardNum = pUser->IsHighCardNum(CardLimit.High_Card,TYPE_CLOVER);
		if( iCardNum != -1 )
		{
			int iPriorty;
			iPriorty = FindCardPriorty(iCardNum,TYPE_CLOVER,pUser);
			if( iPriorty == PRIORTY_A || iPriorty == PRIORTY_B || iPriorty == PRIORTY_C || iPriorty ==  PRIORTY_D )
				iCountGrapedCard++;
			else if( iPriorty == PRIORTY_E )
				iCountCoocockCard++;
			else if( iPriorty == PRIORTY_F )
				iCountGetOutCard++;
		}

	}

	CardLimit = pTable->CheckBoundary(TYPE_HEART);
	if( pUser->IsCloseCard(CardLimit,TYPE_HEART) == TRUE )
	{
		int iCardNum = -1;
		iCardNum = pUser->IsLowCardNum(CardLimit.Low_Card,TYPE_HEART);
		if( iCardNum != -1 )
		{
			int iPriorty;
			iPriorty = FindCardPriorty(iCardNum,TYPE_HEART,pUser);
			if( iPriorty == PRIORTY_A || iPriorty == PRIORTY_B || iPriorty == PRIORTY_C || iPriorty ==  PRIORTY_D )
				iCountGrapedCard++;
			else if( iPriorty == PRIORTY_E )
				iCountCoocockCard++;
			else if( iPriorty == PRIORTY_F )
				iCountGetOutCard++;
		}

		iCardNum = -1;
		iCardNum = pUser->IsHighCardNum(CardLimit.High_Card,TYPE_HEART);
		if( iCardNum != -1 )
		{
			int iPriorty;
			iPriorty = FindCardPriorty(iCardNum,TYPE_HEART,pUser);
			if( iPriorty == PRIORTY_A || iPriorty == PRIORTY_B || iPriorty == PRIORTY_C || iPriorty ==  PRIORTY_D )
				iCountGrapedCard++;
			else if( iPriorty == PRIORTY_E )
				iCountCoocockCard++;
			else if( iPriorty == PRIORTY_F )
				iCountGetOutCard++;
		}
	}

	if ( iCountGrapedCard == 0 && iCountCoocockCard == 0 && iCountGetOutCard == 0 )
		return TRUE; //but this can not be happened
     
	if( IMustPass(iCountGrapedCard,iCountCoocockCard,iCountGetOutCard,pUser) == TRUE )
	{
		return TRUE;
	}
	return FALSE;
}

BOOL SGAlgorithm::IMustPass(int iCountGrapedCard,int iCountCoocockCard,int iCountGetOutCard,SGUser *pUser)
{
// Probiblity Calculation
	int iProbiblity = 0;
	BOOL bPass = FALSE;
	int iPassTickLimit = pUser->GetPassTicketCnt();
	if( iPassTickLimit == 0 )
		return FALSE;

	if( iPassTickLimit == 1 )
	{
		iProbiblity += 50;
		if(iCountGetOutCard > 0 )
		{
			return FALSE;
		}
		else if(iCountCoocockCard > 0 )
		{
			iProbiblity += 10;
			if( iCountGrapedCard > 0 )
				iProbiblity += 10;

			bPass = CheckOtherUserWithProb(pUser, iProbiblity);
		}
		else if(iCountGrapedCard > 0 )
		{
			bPass = CheckOtherUserWithProb(pUser, iProbiblity);
		}
	}
	else if( iPassTickLimit == 2 )
	{
		iProbiblity += 50;
		if(iCountGetOutCard > 0 )
		{
			return FALSE;
		}
		else if(iCountCoocockCard > 0 )
		{
			iProbiblity += 10;
			if( iCountGrapedCard > 0 )
				iProbiblity += 10;

				bPass = CheckOtherUserWithProb(pUser, iProbiblity);
		}
		else if(iCountGrapedCard > 0 )
		{
			iProbiblity += 30;
			bPass = CheckOtherUserWithProb(pUser, iProbiblity);
		}
	}
	else if( iPassTickLimit == 3 )
	{
		iProbiblity += 80;
		if(iCountGetOutCard > 0 )
		{
			return FALSE;
		}
		else if(iCountCoocockCard > 0 )
		{
			iProbiblity += 10;
			if( iCountGrapedCard > 0 )
				iProbiblity += 10;

			bPass = CheckOtherUserWithProb(pUser, iProbiblity);
		}
		else if(iCountGrapedCard > 0 )
		{
			return TRUE;
		}
	}

	return bPass;
}

BOOL SGAlgorithm::CheckOtherUserWithProb(SGUser *pUser, int iProb)
{
	SGManager *pManager = pUser->GetGameManager();		
	int iCheckPass = 0;

	if(pManager == NULL)
	{
		return FALSE;
	}
	std::vector<SGUser*> userlist = pManager->m_userlist;
	

	vector<SGUser*>::iterator pos  = userlist.begin();
	while ( pos != userlist.end())
	{
		SGUser* pOtherUser = (SGUser*)(*pos);

		int iOtherUserID = pOtherUser->GetID();
		int UserID = pUser->GetID();

		if(iOtherUserID != UserID)
		{
			if( pUser->GetPassTicketCnt() < pOtherUser->GetPassTicketCnt() )
			{
				iCheckPass++;
			}
		}
		
		pos++;
	} 

	if(iCheckPass == 0 )
	{
		iProb += 30;
	}
	else if( iCheckPass < (userlist.size() / 3) )
	{
		iProb += 10;
	}

	if(iProb >= 100)
		return TRUE;
	
	return FALSE;

}

BOOL SGAlgorithm::CheckProperCard(SGTable *pTable, SGUser *pUser)
{
	EdgeCard CardLimit;

	CardLimit = pTable->CheckBoundary(TYPE_SPADE);
	if( pUser->IsCloseCard(CardLimit,TYPE_SPADE) == TRUE )
	{
		return TRUE;
	}
	
	CardLimit = pTable->CheckBoundary(TYPE_DIAMOND);		
	if( pUser->IsCloseCard(CardLimit,TYPE_DIAMOND) == TRUE )
	{
		return TRUE;
	}
	
	
	CardLimit = pTable->CheckBoundary(TYPE_CLOVER);		
	if( pUser->IsCloseCard(CardLimit,TYPE_CLOVER) == TRUE )
	{
		return TRUE;
	}

	CardLimit = pTable->CheckBoundary(TYPE_HEART);
	if( pUser->IsCloseCard(CardLimit,TYPE_HEART) == TRUE )
	{
		return TRUE;
	}

	return FALSE;
}
