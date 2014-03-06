//-----------------------------------------------------------------------------
// The primary engine header file. This file links the entire engine together
// and is the only header file that needs to be included in any project using
// the engine.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef ENGINE_H
#define ENGINE_H

//-----------------------------------------------------------------------------
// DirectInput Version Define
//-----------------------------------------------------------------------------
#define DIRECTINPUT_VERSION 0x0800

//-----------------------------------------------------------------------------
// System Includes
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <tchar.h>
#include <windowsx.h>

//-----------------------------------------------------------------------------
// DirectX Includes
//-----------------------------------------------------------------------------
#include <d3dx9.h>
#include <dinput.h>
//#include <dplay8.h>
#include <dmusici.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#define SAFE_DELETE( p )       { if( p ) { delete ( p );     ( p ) = NULL; } }
#define SAFE_DELETE_ARRAY( p ) { if( p ) { delete[] ( p );   ( p ) = NULL; } }
#define SAFE_RELEASE( p )      { if( p ) { ( p )->Release(); ( p ) = NULL; } }

//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#include "Resource.h"
#include "LinkedList.h"
#include "ResourceManagement.h"
#include "Geometry.h"
#include "Font.h"
#include "Scripting.h"
#include "DeviceEnumeration.h"
#include "Input.h"
#include "Network.h"
//#include "SoundSystem.h"
#include "BoundingVolume.h"
#include "Material.h"
#include "Mesh.h"
#include "SceneObject.h"
#include "AnimatedObject.h"
#include "SpawnerObject.h"
#include "ViewFrustum.h"
#include "RenderCache.h"
#include "SceneManager.h"
#include "CollisionDetection.h"
#include "State.h"

//-----------------------------------------------------------------------------
// Engine Setup Structure
//-----------------------------------------------------------------------------
struct EngineSetup
{
	HINSTANCE instance; // Application instance handle.
	GUID guid; // Application GUID.
	char *name; // Name of the application.
	float scale; // Unit scale in meters/unit.
	unsigned char totalBackBuffers; // Number of back buffers to use.
	//void (*HandleNetworkMessage)( ReceivedMessage *msg ); // Network message handler.
	void (*StateSetup)(); // State setup function.
	void (*CreateMaterialResource)( Material **resource, char *name, char *path ); // Material resource creation function.
	char *spawnerPath; // Path for locating the spawner object scripts.

	//-------------------------------------------------------------------------
	// The engine setup structure constructor.
	//-------------------------------------------------------------------------
	EngineSetup()
	{
		GUID defaultGUID = { 0x24215591, 0x24c0, 0x4316, { 0xb5, 0xb2, 0x67, 0x98, 0x2c, 0xb3, 0x82, 0x54 } };

		instance = NULL;
		guid = defaultGUID;
		name = "Application";
		scale = 1.0f;
		totalBackBuffers = 1;
		//HandleNetworkMessage = NULL;
		StateSetup = NULL;
		CreateMaterialResource = NULL;
		spawnerPath = "./";
	}
};

//-----------------------------------------------------------------------------
// Engine Class
//-----------------------------------------------------------------------------
class Engine
{
public:
	Engine( EngineSetup *setup = NULL );
	virtual ~Engine();

	void Run();

	HWND GetWindow();
	void SetDeactiveFlag( bool deactive );

	float GetScale();
	IDirect3DDevice9 *GetDevice();
	D3DDISPLAYMODE *GetDisplayMode();
	ID3DXSprite *GetSprite();

	void AddState( State *state, bool change = true );
	void RemoveState( State *state );
	void ChangeState( unsigned long id );
	State *GetCurrentState();

	ResourceManager< Script > *GetScriptManager();
	ResourceManager< Material > *GetMaterialManager();
	ResourceManager< Mesh > *GetMeshManager();

	Input *GetInput();
	
//20120617
	void SetLocalID(int LocalID){m_LocalID = LocalID;}
	int GetLocalID(){return m_LocalID;}

	void SetPlayerID(int PlayerID){m_PlayerID = PlayerID;}
	int GetPlayerID(){return m_PlayerID;}

//	SoundSystem *GetSoundSystem();
	SceneManager *GetSceneManager();
	IDirect3DStateBlock9* GetGameStateBlock(){return m_pGameStateBlock;}

private:
	bool m_loaded; // Indicates if the engine is loading.
	HWND m_window; // Main window handle.
	bool m_deactive; // Indicates if the application is active or not.

	EngineSetup *m_setup; // Copy of the engine setup structure.
	IDirect3DDevice9 *m_device; // Direct3D device interface.
	D3DDISPLAYMODE m_displayMode; // Display mode of the current Direct3D device.
	ID3DXSprite *m_sprite; // Sprite interface for rendering 2D textured primitives.
	unsigned char m_currentBackBuffer; // Keeps track of which back buffer is at the front of the swap chain.

	LinkedList< State > *m_states; // Linked list of states.
	State *m_currentState; // Pointer to the current state.
	bool m_stateChanged; // Indicates if the state changed in the current frame.

	ResourceManager< Script > *m_scriptManager; // Script manager.
	ResourceManager< Material > *m_materialManager; // Material manager.
	ResourceManager< Mesh > *m_meshManager; // Mesh manager.

	Input *m_input; // Input object.
	int m_LocalID;
	int m_PlayerID;

	//SoundSystem *m_soundSystem; // Sound system.
	SceneManager *m_sceneManager; // Scene manager.
	IDirect3DStateBlock9* m_pGameStateBlock;
};

//-----------------------------------------------------------------------------
// Externals
//-----------------------------------------------------------------------------
extern Engine *g_engine;

#endif