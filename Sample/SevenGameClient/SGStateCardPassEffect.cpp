#include "StdAfx.h"
#include "SGStateCardPassEffect.h"
#include "DirectXSystem.h"
#include "SGManager.h"
#include "SGUser.h"

SGStateCardPassEffect::SGStateCardPassEffect( SGManager* pManager )
: SGState(pManager)
, m_pCurrentUser(NULL)
{

}

SGStateCardPassEffect::~SGStateCardPassEffect(void)
{
}

BOOL SGStateCardPassEffect::OnEnter()
{
	GetSGManager()->SetCardEffectTime(0.0f);
	
	return TRUE;
}

BOOL SGStateCardPassEffect::OnRender( float fElapsedTime )
{
	CDirectXSystem* pRenderer = CDirectXSystem::GetInstance();
	SGManager* pManager = GetSGManager();

	if(pRenderer)
	{
		pRenderer->DrawCard(fElapsedTime);

		if(m_pCurrentUser->GetPassTicketNum() < 0 && m_pCurrentUser->GetStatus() == GAME_END)
			pRenderer->OnRender(fElapsedTime, PICTUREBOX_NOPASS);
		else
			pRenderer->OnRender(fElapsedTime, PICTUREBOX_PASS);

		pRenderer->DrawPassButton(fElapsedTime);
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
		pManager->ChangeState(ENUM_SGCARDTURN);	
	}


	return TRUE;
}