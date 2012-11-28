#include "GUIFSM.h"
#include "GUILogin.h"
#include "GUILobby.h"
#include "GUIRoom.h"
#include "GUIGame.h"

GUIFSM::GUIFSM(eGUIState State)
: GUIState(State)
{
}

GUIFSM::~GUIFSM(void)
{
}

bool GUIFSM::ProcessInput( int InputParam )
{
	return m_pCurrentState->ProcessInput(InputParam);
}

bool GUIFSM::Initialize()
{
	AddState(GUI_STATE_LOGIN);
	AddState(GUI_STATE_LOBBY);
	AddState(GUI_STATE_ROOM);
	AddState(GUI_STATE_GAME);

	GUIStateMap::iterator iter = m_GUIStateMap.find(GUI_STATE_LOGIN);

	m_pCurrentState = iter->second;
	m_pCurrentState->OnEnter();

	return true;
}

bool GUIFSM::AddState ( eGUIState State )
{
	switch (State)
	{
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