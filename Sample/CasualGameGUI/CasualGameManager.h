#pragma once
#include <d3d9.h>

class NetworkSystem;
class CasualGameGUI;

class CasualGameManager
{
public:
	CasualGameManager(void);
	virtual ~CasualGameManager(void);

	bool Initialize(LPDIRECT3DDEVICE9 pDevice);
	bool Finally();

	bool Render(float fElapsedTime);
	bool Update();
	bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam);

	CasualGameGUI* GetGUI(){return m_pGUISystem;}
	NetworkSystem* GetNetwork(){return m_pNetworkSystem;}

	void SetMyID(int serial){m_serial = serial;}
	int GetMyID(){return m_serial;}

private:
	NetworkSystem* m_pNetworkSystem;
	CasualGameGUI* m_pGUISystem;

	int m_serial;
};

