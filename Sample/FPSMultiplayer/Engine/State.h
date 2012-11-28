//-----------------------------------------------------------------------------
// Base state functionality. Applications must derive new states from this
// class to add to the engine.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef STATE_H
#define STATE_H

struct NetworkMessage;
class BasePacket;

//-----------------------------------------------------------------------------
// Viewer Setup Structure
//-----------------------------------------------------------------------------
struct ViewerSetup
{
	SceneObject *viewer; // Current viewing object.
	unsigned long viewClearFlags; // Flags used for clearing the view.

	//-------------------------------------------------------------------------
	// The viewer setup structure constructor.
	//-------------------------------------------------------------------------
	ViewerSetup()
	{
		viewer = NULL;
		viewClearFlags = 0;
	};
};

//-----------------------------------------------------------------------------
// State Class
//-----------------------------------------------------------------------------
class State
{
public:
	State( unsigned long id = 0 );

	virtual void Load();
	virtual void Close();

	virtual void RequestViewer( ViewerSetup *viewer );
	virtual void Update( float elapsed );
	virtual void Render();

	unsigned long GetID();

	virtual void HandleNetworkMessage(BasePacket* pPacket) = 0;
	virtual void HandleNetworkMessage( NetworkMessage* pMessage ) {}
	virtual bool HandleInput(UINT msg, WPARAM wparam, LPARAM lparam) {return false;}

private:
	unsigned long m_id; // Application defined ID (must be unique for state switching).
};

#endif