#pragma once
#include <d3dx9math.h>

typedef struct tag_CharacterInfo
{
	D3DXVECTOR3 translation; // Player's translation.
	float drive; // Player's drive direction.
	float strafe; // Player's strafe direction.
	bool fire; // Indicate's if the player is firing their weapon.
	float viewTilt; // Player's view tilt (i.e. rotation around the x axis).
	float rotationY; // Player's rotation around the y axis.

	bool IsAlive;
	float health;
	unsigned int DeathTime;

	tag_CharacterInfo()
	{
		IsAlive = false;
		health = 0;
		DeathTime = 0;
	}

}_CharacterInfo;

typedef struct tag_GamePlayContents
{

}_GamePlayContents;

typedef struct tag_GameModeInfo
{
	int GameMode;

	tag_GameModeInfo()
	{
		GameMode = -1;
	}

	void SetGameMode(int Mode)
	{
		GameMode = Mode;
	}

}_GameModeInfo;


//////////////////////////////////////////////////////////////
//20120619 GameContents
//////////////////////////////////////////////////////////////
typedef struct tag_SpawnInfo
{
	int SpawnID;

}_SpawnInfo;
