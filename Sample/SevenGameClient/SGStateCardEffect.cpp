#include "StdAfx.h"
#include "SGStateCardEffect.h"
#include "SGManager.h"
#include "DirectXSystem.h"
#include "SGUser.h"

SGStateCardEffect::SGStateCardEffect( SGManager* pManager )
: SGState(pManager)
{
}

SGStateCardEffect::~SGStateCardEffect(void)
{
}

BOOL SGStateCardEffect::OnRender( float fElapsedTime )
{
	CDirectXSystem* pRenderer = CDirectXSystem::GetInstance();
	SGManager* pManager = GetSGManager();

	if(pRenderer)
	{
		pRenderer->DrawCard(fElapsedTime);	
		pRenderer->DrawPassButton(fElapsedTime);
	}

	float fEffectTime = pManager->GetCardEffectTime();

	//SGUser *pHumanUser = pManager->FindUser(HUMAN_PLAYER);

	float fCardSpriteSpeed = 0.8f;
	//if(pHumanUser->GetStatus() == GAME_END)	
		//fCardSpriteSpeed = 0.2f;

	SCardInfo* pCardInfo = pManager->GetSpriteCard();
	CARD_EFFECT_INFO* pCardEffectInfo = pManager->GetCardEffectInfo();

	float fCurPosX = (float)pCardEffectInfo->iOriX + (float)(pCardEffectInfo->iDestX - pCardEffectInfo->iOriX) * (fEffectTime / fCardSpriteSpeed);
	float fCurPosY = (float)pCardEffectInfo->iOriY + (float)(pCardEffectInfo->iDestY - pCardEffectInfo->iOriY) * (fEffectTime / fCardSpriteSpeed);

	int iCol, iRow;
	pManager->GetCardImageIndex(pCardInfo, iCol, iRow);

	if(fEffectTime < fCardSpriteSpeed)
	{
		pRenderer->OnRenderRegion(fElapsedTime, PICTUREBOX_CARDSET, 
			fCurPosX, 
			fCurPosY,
			((float)(iCol))/CARDNUM_CARDSET_WIDTH,  
			((float)(iRow))/CARDNUM_CARDSET_HEIGHT,
			CARDRATIO_CARDSET_WIDTH,
			CARDRATIO_CARDSET_HEIGHT,
			CARD_WIDTH,
			CARD_HEIGHT);

		fEffectTime += fElapsedTime;
		pManager->SetCardEffectTime(fEffectTime);
	}
	else
	{
		pManager->UpdateTableState(pCardInfo);

		pRenderer->OnRenderRegion(fElapsedTime, PICTUREBOX_CARDSET,
			pCardEffectInfo->iDestX, 
			pCardEffectInfo->iDestY,
			((float)(iCol))/CARDNUM_CARDSET_WIDTH,  
			((float)(iRow))/CARDNUM_CARDSET_HEIGHT,
			CARDRATIO_CARDSET_WIDTH,
			CARDRATIO_CARDSET_HEIGHT,
			CARD_WIDTH,
			CARD_HEIGHT);

		pManager->ChangeState(ENUM_SGCARDTURN);
	}

	return TRUE;
}

BOOL SGStateCardEffect::OnEnter()
{
	GetSGManager()->SetCardEffectTime(0.0f);
	return TRUE;
}

