#pragma once
#include <map>
#include "GUIState.h"

class GUIFSM : public GUIState
{
	typedef std::map<eGUIState, GUIState*> GUIStateMap;
public:
	GUIFSM(eGUIState State);
	virtual ~GUIFSM(void);

	bool Initialize();
	bool AddState( eGUIState State );
	bool ChangeState(eGUIState State);

	virtual bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual bool Notify(BasePacket* pPacket) override;
	virtual void OnRender(float fElapsedTime) {if(m_pCurrentState) m_pCurrentState->OnRender(fElapsedTime);}
	virtual bool handleRootKeyDown(const CEGUI::EventArgs& args){return m_pCurrentState->handleRootKeyDown(args);}

	eGUIState GetGUIState(){return m_pCurrentState->GetGUIState();}

protected:

private:
	GUIState* m_pCurrentState;
	GUIStateMap m_GUIStateMap;
};
