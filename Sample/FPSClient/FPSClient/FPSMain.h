#pragma once

#include "resource.h"
//-----------------------------------------------------------------------------
// System Includes
//-----------------------------------------------------------------------------

#include <windows.h>

//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#include "../FPSEngine/Engine.h"
#pragma comment( lib, "FPSEngine.lib" )

//-----------------------------------------------------------------------------
// Game Includes
//-----------------------------------------------------------------------------
#include "GameMaterial.h"
#include "Bullet.h"
#include "Weapon.h"
#include "PlayerObject.h"
#include "PlayerManager.h"
#include "Wait.h"
#include "Game.h"
#include "GamePacketStructure.h"

//-----------------------------------------------------------------------------
// Player Data Structure
//-----------------------------------------------------------------------------
struct PlayerData
{
	char character[MAX_PATH];
	char map[MAX_PATH];
};
