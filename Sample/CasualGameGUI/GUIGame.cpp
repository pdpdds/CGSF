#include "stdafx.h"
#include "GUIGame.h"
#include "PacketID.h"
#include "SFPacketStore.pb.h"
#include "BasePacket.h"
#include "SFProtobufPacket.h"
#include "CasualGameManager.h"
#include "NetworkSystem.h"
#include "CasualGameGUI.h"

using namespace google;
using namespace CEGUI;

extern CasualGameManager* g_pCasualGameManager;

extern IGameMain* g_pGameMain;

GUIGame::GUIGame(eGUIState State)
: GUIState(State)
{

}

GUIGame::~GUIGame(void)
{
}

bool GUIGame::ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	g_pGameMain->ProcessInput(uMsg, wParam, lParam);
	return true;
}

bool GUIGame::OnEnter()
{
	CEGUI::Window* parent = NULL;

	// decide where to attach the console main window
	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->setProperty("BackgroundEnabled", "false");
	
	g_pGameMain->Initialize();
	return true;
}

bool GUIGame::OnLeave()
{
	CEGUI::Window* parent = NULL;

	// decide where to attach the console main window
	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->setProperty("BackgroundEnabled", "true");

	g_pGameMain->Finally();
	return true;
}

bool GUIGame::Notify(BasePacket* pPacket)
{
	g_pGameMain->Notify(pPacket);
	return true;
}

void GUIGame::OnRender(float fElapsedTime)
{
	g_pGameMain->OnRender(fElapsedTime);
}


bool GUIGame::handleRootKeyDown(const CEGUI::EventArgs& args)
{
	const KeyEventArgs& keyArgs = static_cast<const KeyEventArgs&>(args);

	switch (keyArgs.scancode)
	{
	case Key::Return:
		{
			SFProtobufPacket<SFPacketStore::LeaveRoom> pktLeaveRoom(CGSF::LeaveRoom);
			g_pCasualGameManager->GetNetwork()->TCPSend(&pktLeaveRoom);			
		}
		break;

	default:
		return false;
	}

	return true;
}