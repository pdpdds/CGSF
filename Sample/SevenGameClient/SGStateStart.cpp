#include "StdAfx.h"
#include "SGStateStart.h"
#include "SGManager.h"
#include "DirectXSystem.h"

SGStateStart::SGStateStart( SGManager* pManager )
: SGState(pManager)
{
}

SGStateStart::~SGStateStart(void)
{
}

BOOL SGStateStart::OnEnter()
{
	GetSGManager()->SetCardEffectTime(0.0f);
	return TRUE;
}

BOOL SGStateStart::OnRender( float fElapsedTime )
{
	CDirectXSystem* pRenderer = CDirectXSystem::GetInstance();
	SGManager* pManager = GetSGManager();

	if(pRenderer)
	{
		pRenderer->DrawCard(fElapsedTime);	
		pRenderer->OnRender(fElapsedTime, PICTUREBOX_GAMESTART);
		pRenderer->DrawPassButton(fElapsedTime);
	}

	float fEffectTime = pManager->GetCardEffectTime();

	float fModeTime = 2.0f;

	if(fEffectTime < fModeTime)
	{

		fEffectTime += fElapsedTime;
		pManager->SetCardEffectTime(fEffectTime);
	}
	else
	{
		pManager->ChangeState(ENUM_SGGAME);	
	}

	return TRUE;
}
