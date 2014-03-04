#pragma once
#include "GUIState.h"
#include "IGameMain.h"

class GUIGame : public GUIState
{
public:
	GUIGame(eGUIState State);
	virtual ~GUIGame(void);

	bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual bool OnEnter() override;
	virtual bool OnLeave() override;
	virtual bool Notify(BasePacket* pPacket) override;
	virtual void OnRender(float fElapsedTime) override;

	virtual bool handleRootKeyDown(const CEGUI::EventArgs& args) override;
protected:

private:
};
