//-----------------------------------------------------------------------------
// A very basic menu state that loads (and handles messages for) a simple
// dialog box for the menu system.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef MENU_H
#define MENU_H

//-----------------------------------------------------------------------------
// State ID Define
//-----------------------------------------------------------------------------
#define STATE_MENU 0

//-----------------------------------------------------------------------------
// Menu Class
//-----------------------------------------------------------------------------
class Menu : public State
{
public:
	Menu();

	virtual void Update( float elapsed );
	virtual void HandleNetworkMessage(int PacketID, BYTE* pBuffer, USHORT Length) override;
	virtual void HandleNetworkMessage( NetworkMessage* pMessage ) override;

	virtual void Render() override;
	virtual bool HandleInput(UINT msg, WPARAM wparam, LPARAM lparam) override;

};

#endif