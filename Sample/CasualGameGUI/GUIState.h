#pragma once
#include <CEGUI/CEGUI.h>

class BasePacket;

typedef enum eGUIState
{
	GUI_STATE_NONE = 0,
	GUI_STATE_INIT,
	GUI_STATE_LOGIN,
	GUI_STATE_LOBBY,
	GUI_STATE_ROOM,
	GUI_STATE_LOADING,
	GUI_STATE_GAME,
};

class GUIState
{
public:
	GUIState(eGUIState State){m_State = State;}
	virtual ~GUIState(void){}

	virtual bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

	virtual bool Notify(BasePacket* pPacket) {return false;}
	eGUIState GetGUIState(){return m_State;}
	virtual void OnRender(float fElapsedTime){}

	virtual bool OnEnter(){return false;}
	virtual bool OnLeave(){return false;}

	virtual bool handleRootKeyDown(const CEGUI::EventArgs& args){return false;}

protected:

private:
	eGUIState m_State;
};