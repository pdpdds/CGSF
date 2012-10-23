#include "GUIGame.h"

GUIGame::GUIGame(eGUIState State)
: GUIState(State)
{

}

GUIGame::~GUIGame(void)
{
}

bool GUIGame::ProcessInput( int InputParam )
{
	return true;
}
