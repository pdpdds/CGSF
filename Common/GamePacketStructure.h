#pragma once


#define PLAYERID int 

#pragma pack(push, 1)

//-----------------------------------------------------------------------------
// Network Message Structure
//-----------------------------------------------------------------------------
struct NetworkMessage
{
	unsigned long msgid; // Message ID.
	PLAYERID PlayerID; // PlayerID
};

//-----------------------------------------------------------------------------
// Received Message Structure
//-----------------------------------------------------------------------------
struct ReceivedMessage : public NetworkMessage
{
	char data[32]; // Message data.
};

//-----------------------------------------------------------------------------
// Player Health Message Structure
//-----------------------------------------------------------------------------
struct PlayerHealthMsg : public NetworkMessage
{
	float health; // Absolute health of the player.
};

//-----------------------------------------------------------------------------
// Player Move Message Structure
//-----------------------------------------------------------------------------
struct PlayerMoveUpdateMsg : public NetworkMessage
{
	D3DXVECTOR3 translation; // Player's translation.
	float drive; // Player's drive direction.
	float strafe; // Player's strafe direction.
	bool fire; // Indicate's if the player is firing their weapon.
};

//-----------------------------------------------------------------------------
// Player Look Update Message Structure
//-----------------------------------------------------------------------------
struct PlayerLookUpdateMsg : public NetworkMessage
{
	float viewTilt; // Player's view tilt (i.e. rotation around the x axis).
	float rotationY; // Player's rotation around the y axis.
};

//-----------------------------------------------------------------------------
// Player Score Message Structure
//-----------------------------------------------------------------------------
struct PlayerScoreMsg : public NetworkMessage
{
	unsigned long frags; // Player's frag count.
	unsigned long deaths; // Player's death tally.
};

//-----------------------------------------------------------------------------
// Player Weapon Change Message Structure
//-----------------------------------------------------------------------------
struct PlayerWeaponChangeMsg : public NetworkMessage
{
	char weapon; // Weapon that the player has changed to.
};

//-----------------------------------------------------------------------------
// Spawn Point Message Structure
//-----------------------------------------------------------------------------
struct SpawnPointMsg : public NetworkMessage
{
	long spawnPoint; // ID of the spawn point to use.
};

//-----------------------------------------------------------------------------
// Spawn Point Message Structure
//-----------------------------------------------------------------------------
struct SpawnPlayerMsg : public NetworkMessage
{
	D3DXVECTOR3 translation; // Translation to spawn the player at.
};

struct PlayerIPMsg : public NetworkMessage
{
	DWORD LocalIP;
	USHORT LocalPort;
	DWORD ExternalIP;
	USHORT ExternalPort;
};

#pragma pack(pop)