#pragma once
#include "GUIState.h"
#include <CEGUI/CEGUI.H>

class GUIRoom : public GUIState
{
public:
	GUIRoom(eGUIState State);
	~GUIRoom();

	virtual bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual bool OnEnter() override;
	virtual bool OnLeave() override;

	bool isVisible() const;

	virtual bool Notify(BasePacket* pPacket) override;

private:
	bool AddMessage(std::string& message);

	bool handleSend(const CEGUI::EventArgs& args);
	bool handleStart(const CEGUI::EventArgs& args);
	bool handleTeamChange(const CEGUI::EventArgs& args);
	bool handleLeave(const CEGUI::EventArgs& args);

	bool handleKeyDown(const CEGUI::EventArgs& args);

	CEGUI::Window* d_root;
	int d_historyPos;
	std::vector<CEGUI::String> d_history;
};