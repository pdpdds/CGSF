#pragma once
#include <d3dx9.h>
#include "GUIState.h"
//#include "CEGuiSample.h"
#include <CEGUI/CEGUI.h>

class GUILobby;
class GUIFSM;

class GUIManager// : public CEGuiSample
{
public:
	GUIManager(void);
	virtual ~GUIManager(void);

	bool ChangeState( eGUIState State );
	bool ProcessInput( int InputParam );
	virtual bool Notify(BasePacket* pPacket);

	bool Create(IDirect3DDevice9* pDevice);
	bool handleRootKeyDown(const CEGUI::EventArgs& args);
	bool Render();
	
	

protected:

protected:
	GUIFSM* m_pGUIFSM;
	GUILobby* m_LobbyConsole;
};
