#pragma once

#include "GUIState.h"
//#include "CEGuiSample.h"
#include "CEGUI.h"

class GUILogin : public GUIState
{
public:
	GUILogin(eGUIState State);
	~GUILogin();

	virtual bool ProcessInput(int InputParam) override;
	virtual bool OnEnter() override;
	virtual bool OnLeave() override;

	bool isVisible() const;

	virtual bool Notify(int Msg, char* pBuffer, int BufferSize ) override;

private:
	// these must match the IDs assigned in the layout
	static const unsigned int NameEditBoxID;
	static const unsigned int PasswordEditBoxID;
	static const unsigned int SubmitButtionID;
	
	bool handleSubmit(const CEGUI::EventArgs& args);
	bool handleKeyDown(const CEGUI::EventArgs& args);

	CEGUI::Window* d_root;
	int d_historyPos;
	std::vector<CEGUI::String> d_history;
};
