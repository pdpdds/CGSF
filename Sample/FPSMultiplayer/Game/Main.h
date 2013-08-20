//-----------------------------------------------------------------------------
// The main entry point. This file links the game to the engine.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef MAIN_H
#define MAIN_H

//-----------------------------------------------------------------------------
// System Includes
//-----------------------------------------------------------------------------

#include <windows.h>

//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#include "../Engine/Engine.h"
#pragma comment( lib, "Engine.lib" )

//-----------------------------------------------------------------------------
// Game Includes
//-----------------------------------------------------------------------------
#include "GameMaterial.h"
#include "Bullet.h"
#include "Weapon.h"
#include "PlayerObject.h"
#include "PlayerManager.h"
#include "Menu.h"
#include "Game.h"

//-----------------------------------------------------------------------------
// Player Data Structure
//-----------------------------------------------------------------------------
struct PlayerData
{
	char character[MAX_PATH];
	char map[MAX_PATH];
};

#endif