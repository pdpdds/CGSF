#include "StdAfx.h"
#include "SGStateManager.h"
#include "SGStateCardEffect.h"
#include "SGStateCardPassEffect.h"
#include "SGStateCardTurn.h"
#include "SGStateGameOver.h"
#include "SGStateGame.h"
#include "SGStateStart.h"

SGStateManager::SGStateManager( SGManager* pManager )
: SGState(pManager)
{
	m_pSGManager = pManager;
	m_pCurrentSGState = NULL;
}

SGStateManager::~SGStateManager(void)
{
}

BOOL SGStateManager::OnMessage(int iX, int iY)
{
	if(m_pCurrentSGState)
		return m_pCurrentSGState->OnMessage(iX, iY);
	
	return FALSE;
}

BOOL SGStateManager::OnRender(float fElapsedTime)
{
	if(m_pCurrentSGState)
		return m_pCurrentSGState->OnRender(fElapsedTime);

	return FALSE;
}

BOOL SGStateManager::ChangeState( eSGState state )
{
	if(!m_pCurrentSGState)
		return FALSE;

	mapSGState::iterator iter = m_mapSGState.find(state);

	if(iter == m_mapSGState.end())
		return FALSE;

	if(iter->second == m_pCurrentSGState)
		return FALSE;

	m_pCurrentSGState->OnLeave();

	m_pCurrentSGState = iter->second;

	m_pCurrentSGState->OnEnter();

	return TRUE;
	
}

BOOL SGStateManager::AddState( eSGState state)
{
	switch(state)
	{
	case ENUM_SGINIT:
		{
			SGState* pState = new SGState(m_pSGManager);
			m_mapSGState.insert(mapSGState::value_type(state, pState));
			m_pCurrentSGState = pState;
		}
		break;
	case ENUM_SGCARDEFFECT:
		{
			SGState* pState = new SGStateCardEffect(m_pSGManager);
			m_mapSGState.insert(mapSGState::value_type(state, pState));
		}
		break;
	case ENUM_SGCARDTURN:
		{
			SGState* pState = new SGStateCardTurn(m_pSGManager);
			m_mapSGState.insert(mapSGState::value_type(state, pState));
		}
		break;
	case ENUM_SGGAME:
		{
			SGState* pState = new SGStateGame(m_pSGManager);
			m_mapSGState.insert(mapSGState::value_type(state, pState));
		}
		break;
	case ENUM_SGSTART:
		{
			SGState* pState = new SGStateStart(m_pSGManager);
			m_mapSGState.insert(mapSGState::value_type(state, pState));
		}
		break;
	case ENUM_SGGAMEOVER:
		{
			SGState* pState = new SGStateGameOver(m_pSGManager);
			m_mapSGState.insert(mapSGState::value_type(state, pState));
		}
		break;
	case ENUM_SGCARDPASSEFFECT:
		{
			SGState* pState = new SGStateCardPassEffect(m_pSGManager);
			m_mapSGState.insert(mapSGState::value_type(state, pState));
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}