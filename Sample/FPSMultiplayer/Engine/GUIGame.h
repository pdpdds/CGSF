#pragma once
#include "GUIState.h"

class GUIGame : public GUIState
{
public:
	GUIGame(eGUIState State);
	virtual ~GUIGame(void);

	bool ProcessInput( int InputParam );

protected:

private:
};
