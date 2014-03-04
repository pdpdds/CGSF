#include "stdafx.h"
#include "GUIFSM.h"
#include "GUIInit.h"
#include "GUILogin.h"
#include "GUILobby.h"
#include "GUIRoom.h"
#include "GUILoading.h"
#include "GUIGame.h"

GUIFSM::GUIFSM(eGUIState State)
: GUIState(State)
{
}

GUIFSM::~GUIFSM(void)
{
}

bool GUIFSM::ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return m_pCurrentState->ProcessInput(uMsg, wParam, lParam);
}

bool GUIFSM::Initialize()
{
	AddState(GUI_STATE_INIT);
	AddState(GUI_STATE_LOGIN);
	AddState(GUI_STATE_LOBBY);
	AddState(GUI_STATE_ROOM);
	AddState(GUI_STATE_LOADING);
	AddState(GUI_STATE_GAME);

	GUIStateMap::iterator iter = m_GUIStateMap.find(GUI_STATE_INIT);

	m_pCurrentState = iter->second;
	m_pCurrentState->OnEnter();

	return true;
}

bool GUIFSM::AddState ( eGUIState State )
{
	switch (State)
	{
		case GUI_STATE_INIT:
		{
			GUIState* pState = new GUIInit(GUI_STATE_INIT);
			m_GUIStateMap.insert(std::make_pair(GUI_STATE_INIT, pState));
		}
		break;
	case GUI_STATE_LOGIN:
		{
			GUIState* pState = new GUILogin(GUI_STATE_LOGIN);
			m_GUIStateMap.insert(std::make_pair(GUI_STATE_LOGIN, pState));
		}
		break;
	case GUI_STATE_LOBBY:
		{
			GUIState* pState = new GUILobby(GUI_STATE_LOBBY);
			m_GUIStateMap.insert(std::make_pair(GUI_STATE_LOBBY, pState));
		}
		break;
	case GUI_STATE_ROOM:
		{
			GUIState* pState = new GUIRoom(GUI_STATE_ROOM);
			m_GUIStateMap.insert(std::make_pair(GUI_STATE_ROOM, pState));
		}
		break;
	case GUI_STATE_GAME:
		{
			GUIState* pState = new GUIGame(GUI_STATE_GAME);
			m_GUIStateMap.insert(std::make_pair(GUI_STATE_GAME, pState));
		}
		break;
	case GUI_STATE_LOADING:
		{
			GUIState* pState = new GUILoading(GUI_STATE_LOADING);
			m_GUIStateMap.insert(std::make_pair(GUI_STATE_LOADING, pState));
		}
		
		break;
	}

	return true;
}

bool GUIFSM::ChangeState( eGUIState State )
{
	if(m_pCurrentState->GetGUIState() == State)
	{
		return false;
	}

	GUIStateMap::iterator iter = m_GUIStateMap.find(State);

	if(iter == m_GUIStateMap.end())
	{
		return false;
	}

	m_pCurrentState->OnLeave();

	m_pCurrentState = iter->second;

	m_pCurrentState->OnEnter();

	return true;
}

bool GUIFSM::Notify(BasePacket* pPacket)
{
	return m_pCurrentState->Notify(pPacket );
}