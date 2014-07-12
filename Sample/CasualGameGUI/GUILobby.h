#pragma once
#include "GUIState.h"

class GUILobby : public GUIState
{
public:
	GUILobby(eGUIState State);
	~GUILobby();

	virtual bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual bool OnEnter() override;
	virtual bool OnLeave() override;

	bool isVisible() const;

	virtual bool Notify(BasePacket* pPacket) override;

private:
	bool handleSubmit(const CEGUI::EventArgs& args);
	bool handleRoomCreate(const CEGUI::EventArgs& args);
	bool handleRoomListRefresh( const CEGUI::EventArgs& args );
	bool handleRoomClick( const CEGUI::EventArgs& args );

	bool handleKeyDown(const CEGUI::EventArgs& args);

	CEGUI::Window* d_root;
	int d_historyPos;
	std::vector<CEGUI::String> d_history;
};
