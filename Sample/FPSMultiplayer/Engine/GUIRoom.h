#pragma once
#include "GUIState.h"
//#include "CEGuiSample.h"
#include "CEGUI.h"

class GUIRoom : public GUIState
{
public:
	GUIRoom(eGUIState State);
	~GUIRoom();

	virtual bool ProcessInput(int InputParam) override;
	virtual bool OnEnter() override;
	virtual bool OnLeave() override;

	bool isVisible() const;

	virtual bool Notify(int Msg, char* pBuffer, int BufferSize ) override;

private:
	// these must match the IDs assigned in the layout
	static const unsigned int SendButtonID;
	static const unsigned int EntryBoxID;
	static const unsigned int HistoryID;
	static const unsigned int StartButtonID;
	static const unsigned int TeamChangeButtonID;
	static const unsigned int LeaveButtonID;

	bool handleSend(const CEGUI::EventArgs& args);
	bool handleStart(const CEGUI::EventArgs& args);
	bool handleTeamChange(const CEGUI::EventArgs& args);
	bool handleLeave(const CEGUI::EventArgs& args);

	bool handleKeyDown(const CEGUI::EventArgs& args);

	CEGUI::Window* d_root;
	int d_historyPos;
	std::vector<CEGUI::String> d_history;
};