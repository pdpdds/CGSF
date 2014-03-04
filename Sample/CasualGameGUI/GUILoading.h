#pragma once
#include "GUIState.h"
#include <CEGUI/CEGUI.h>

class GUILoading : public GUIState
{
public:
	GUILoading(eGUIState State);
	virtual ~GUILoading(void);

	virtual bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual bool OnEnter() override;
	virtual bool OnLeave() override;

	virtual bool Notify(BasePacket* pPacket) override;
	virtual void OnRender(float fElapsedTime) override;

	bool handleUpdate(const CEGUI::EventArgs& args);

private:
	CEGUI::Window* d_root;
	float m_loadingTime;
	bool m_bReportLoadingComplete;
};