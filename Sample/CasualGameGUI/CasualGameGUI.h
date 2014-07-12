#pragma once
#include <d3d9.h>
#include "GUIState.h"


class BasePacket;
class GUIFSM;
class GUILobby;

class CasualGameGUI
{
public:
	CasualGameGUI(void);
	virtual ~CasualGameGUI(void);

	bool ChangeState( eGUIState State );
	bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual bool Notify(BasePacket* pPacket);

	bool Create(IDirect3DDevice9* pDevice);
	bool handleRootKeyDown(const CEGUI::EventArgs& args);
	bool Render(float fElapsedTime);
	bool OnResetDevice(LPDIRECT3DDEVICE9 pDevice);
	void PopupMessage(char* szMessage);
	void QuitMessage(char* szMessage);

	eGUIState GetGUIState();
	
protected:

protected:
	GUIFSM* m_pGUIFSM;
	GUILobby* m_LobbyConsole;
};

