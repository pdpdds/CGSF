#pragma once
#include "GUIState.h"
//#include "CEGuiSample.h"
#include <CEGUI/CEGUI.h>

class GUILobby : public GUIState
{
public:
	GUILobby(eGUIState State);
	~GUILobby();

	virtual bool ProcessInput(int InputParam) override;
	virtual bool OnEnter() override;
	virtual bool OnLeave() override;

	bool isVisible() const;

	virtual bool Notify(BasePacket* pPacket) override;

private:
	// these must match the IDs assigned in the layout
	static const unsigned int SubmitButtonID;
	static const unsigned int EntryBoxID;
	static const unsigned int HistoryID;
	static const unsigned int CreateButtonID;
	static const unsigned int JoinButtonID;

	bool handleSubmit(const CEGUI::EventArgs& args);
	bool handleRoomCreate(const CEGUI::EventArgs& args);
	bool handleRoomJoin(const CEGUI::EventArgs& args);
	
	bool handleKeyDown(const CEGUI::EventArgs& args);

	CEGUI::Window* d_root;
	int d_historyPos;
	std::vector<CEGUI::String> d_history;
};
