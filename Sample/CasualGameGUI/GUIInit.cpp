#include "stdafx.h"
#include "GUIInit.h"


GUIInit::GUIInit(eGUIState State)
: GUIState(State)
{
}


GUIInit::~GUIInit(void)
{
}

bool GUIInit::OnEnter()
{
	return true;
}

bool GUIInit::OnLeave()
{
	return true;
}

bool GUIInit::ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return true;
}

bool GUIInit::Notify(BasePacket* pPacket) 
{
	return true;
}
