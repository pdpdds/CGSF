// FPSMain.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <string>
#include "FPSMain.h"
#include "Wait.h"
#include "Game.h"
#include "GameMaterial.h"
#include "CasualGameManager.h"
#include "FPSGameMain.h"
#include "SFGameProtocol.h"
#include "SFPacketProtocol.h"

// TODO: reference additional headers your program requires here
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dinput8.lib")

#ifdef _DEBUG
#pragma comment(lib, "d3dx9d.lib")
#pragma comment(lib, "CEGUIBase-0_d.lib")
#pragma comment(lib, "CEGUIDirect3D9Renderer-0_d.lib")
#else
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "CEGUIBase-0.lib")
#pragma comment(lib, "CEGUIDirect3D9Renderer-0.lib")
#endif

#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "dsound.lib")

#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "liblzf.lib")
#pragma comment(lib, "CasualGameGUI.lib")
#pragma comment(lib, "EngineLayer.lib")
#pragma comment(lib, "BaseLayer.lib")
#pragma comment(lib, "DatabaseLayer.lib")
#pragma comment(lib, "libprotobuf.lib")

IGameMain* g_pGameMain = 0;
CasualGameManager* g_pCasualGameManager = 0;
//-----------------------------------------------------------------------------
// Application specific network message handler.
//-----------------------------------------------------------------------------
/*void HandleNetworkMessage( ReceivedMessage *msg )
{
	// Only allow network messages to be processed while in the game state.
	if( g_engine->GetCurrentState()->GetID() != STATE_GAME )
		return;

	// Pass control over to the game state's network message handler.
	//( (Game*)g_engine->GetCurrentState() )->HandleNetworkMessage( msg );
}*/

//-----------------------------------------------------------------------------
// Application specific state setup.
//-----------------------------------------------------------------------------
void StateSetup()
{
	g_engine->AddState( new Wait, true );
	g_engine->AddState( new Game, false );
}

//-----------------------------------------------------------------------------
// Application specific material creation.
//-----------------------------------------------------------------------------
void CreateMaterialResource( Material **resource, char *name, char *path = "./" )
{
	*resource = new GameMaterial( name, path );
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	// A unique id for the application so it can be identified on a network.
	GUID guid = { 0xd6c55c78, 0x5030, 0x43b7, { 0x85, 0xa9, 0xc, 0x8b, 0xbe, 0x77, 0x5a, 0x62 } };

	// Create the engine setup structure.
	EngineSetup setup;
	setup.instance = hInstance;
	setup.guid = guid;
	setup.name = "Cityscape";
	setup.scale = 0.01f;
	setup.totalBackBuffers = 1;
	//setup.HandleNetworkMessage = HandleNetworkMessage;
	setup.StateSetup = StateSetup;
	setup.CreateMaterialResource = CreateMaterialResource;
	setup.spawnerPath = "./Assets/Objects/";

	g_pGameMain = new FPSGameMain();
	g_pCasualGameManager = new CasualGameManager();

	// Create the engine (using the setup structure), then run it.
	new Engine( &setup );

	/*TCPNetworkCallback* pCallback = new TCPNetworkCallback();
	UDPNetworkCallback* pUDPCallback = new UDPNetworkCallback(); 

	g_engine->GetNetwork()->Initialize("CGSFEngine.dll", pCallback, pUDPCallback);

	IPacketProtocol* pProtocol = new SFPacketProtocol<SFGameProtocol>;
	g_engine->GetNetwork()->SetPacketProtocol(pProtocol);

	ILogicDispatcher* pDispatcher = new SFCasualGameDispatcher();
	g_engine->GetNetwork()->SetLogicDispatcher(pDispatcher);
	*/

	
	
	if(g_engine != NULL)
	{
		IPacketProtocol* pProtocol = new SFPacketProtocol<SFGameProtocol>;
		g_pCasualGameManager->Initialize(g_engine->GetDevice(), pProtocol);
		g_engine->Run();
	}

	return true;
}

