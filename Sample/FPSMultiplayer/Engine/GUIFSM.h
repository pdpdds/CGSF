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

	virtual bool ProcessInput(int InputParam) override;
	virtual bool Notify(BasePacket* pPacket) override;

protected:

private:
	GUIState* m_pCurrentState;
	GUIStateMap m_GUIStateMap;
};
