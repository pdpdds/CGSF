#include "StdAfx.h"
#include "SGStateCardTurn.h"
#include "SGManager.h"
#include "DirectXSystem.h"
#include "SGUser.h"

SGStateCardTurn::SGStateCardTurn( SGManager* pManager )
: SGState(pManager)
{
}

SGStateCardTurn::~SGStateCardTurn(void)
{
}

BOOL SGStateCardTurn::OnRender( float fElapsedTime )
{
	CDirectXSystem* pRenderer = CDirectXSystem::GetInstance();
	SGManager* pManager = GetSGManager();

	if(pRenderer)
	{
		pRenderer->DrawCard(fElapsedTime);	

		pRenderer->DrawPassButton(fElapsedTime);

		pRenderer->DrawCardTurn(fElapsedTime);
	}

	float fEffectTime = pManager->GetCardEffectTime();

	float fModeTime = 0.5f;

	if(fEffectTime < fModeTime)
	{
		fEffectTime += fElapsedTime;
		pManager->SetCardEffectTime(fEffectTime);
	}
	else
	{
		pManager->ChangeState(ENUM_SGGAME);
		return TRUE;
	}

	return TRUE;
}

BOOL SGStateCardTurn::OnEnter()
{
	GetSGManager()->SetCardEffectTime(0.0f);
	return TRUE;
}