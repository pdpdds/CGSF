#include "StdAfx.h"
#include "SGStateGameOver.h"
#include "SGManager.h"
#include "DirectXSystem.h"

SGStateGameOver::SGStateGameOver( SGManager* pManager )
: SGState(pManager)
{
}

SGStateGameOver::~SGStateGameOver(void)
{
}

BOOL SGStateGameOver::OnRender( float fElapsedTime )
{
	CDirectXSystem* pRenderer = CDirectXSystem::GetInstance();

	if(pRenderer)
	{
		pRenderer->DrawCard(fElapsedTime);	
		pRenderer->DrawWinLose(fElapsedTime);
	}

	return TRUE;
}

BOOL SGStateGameOver::OnMessage( int iX, int iY )
{
	//GetSGManager()->InitializeData();
	//return GetSGManager()->ChangeState(ENUM_SGSTART);
	return TRUE;
}

BOOL SGStateGameOver::OnEnter()
{
	CDirectXSystem* pRenderer = CDirectXSystem::GetInstance();
	pRenderer->ResetFireEffect();

	return TRUE;
}