#pragma once
#include <CEGUI/CEGUI.h>
#include "GUIState.h"

class GUIInit : public GUIState
{
public:
	GUIInit(eGUIState State);
	virtual ~GUIInit(void);

	virtual bool OnEnter() override;
	virtual bool OnLeave() override;
	virtual bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual bool Notify(BasePacket* pPacket) override;
};

