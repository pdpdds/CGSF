//-----------------------------------------------------------------------------
// Menu.h implementation.
// Refer to the Menu.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "FPSMain.h"
#include <SFPacketStore/SFPacketID.h>
#include "CommonStructure.h"
#include <SFPacketStore/SFPacketStore.pb.h>
#include "BasePacket.h"
#include "SFProtobufPacket.h"
#include "CasualGameManager.h"

extern CasualGameManager* g_pCasualGameManager;
//-----------------------------------------------------------------------------
// Menu class constructor.
//-----------------------------------------------------------------------------
Wait::Wait() : State( STATE_WAIT )
{
	// Does nothing but set's the state's ID.
}

//-----------------------------------------------------------------------------
// Update the menu state.
//-----------------------------------------------------------------------------
void Wait::Update( float elapsed )
{
	// Update the Casual Game object, processing any pending messages.
	g_pCasualGameManager->Update();
}

void Wait::HandleNetworkMessage(BasePacket* pPacket)
{

}

void Wait::HandleNetworkMessage( NetworkMessage* pMessage )
{
    /*if (pMessage->msgid == CGSF::PlayerIP)
	{
		g_engine->TCPSend(CGSF::PlayerIP, pMessage, sizeof(PlayerIPMsg));
	}*/

}

void Wait::Render(float elapsed)
{
	g_pCasualGameManager->Render(elapsed);
}

bool Wait::HandleInput( UINT msg, WPARAM wparam, LPARAM lparam )
{
	if(g_pCasualGameManager)				
		g_pCasualGameManager->ProcessInput(msg, wparam, lparam);

	return false;
}
