#pragma once

typedef enum eGUIState
{
	GUI_STATE_NONE = 0,
	GUI_STATE_LOGIN,
	GUI_STATE_LOBBY,
	GUI_STATE_ROOM,
	GUI_STATE_GAME,
};

class GUIState
{
public:
	GUIState(eGUIState State){m_State = State;}
	virtual ~GUIState(void){}

	virtual bool ProcessInput(int InputParam) = 0;
	virtual bool Notify(int Msg, char* pBuffer, int BufferSize ) {return false;}
	eGUIState GetGUIState(){return m_State;}

	virtual bool OnEnter(){return false;}
	virtual bool OnLeave(){return false;}

protected:

private:
	eGUIState m_State;
};