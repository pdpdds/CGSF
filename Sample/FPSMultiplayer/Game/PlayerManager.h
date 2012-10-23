//-----------------------------------------------------------------------------
// Manages the player's in the game. More specifically it manages The local
// player and its input.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

//-----------------------------------------------------------------------------
// Player Manager Class
//-----------------------------------------------------------------------------
class PlayerManager
{
public:
	PlayerManager();
	virtual ~PlayerManager();

	void Update( float elapsed );

	void SpawnLocalPlayer( long spawnPoint = -1 );
	void SpawnPlayer( int dpnid, D3DXVECTOR3 translation );

	PlayerObject *AddPlayer( PlayerInfo *player );
	void RemovePlayer( int dpnid );

	PlayerObject *GetLocalPlayer();
	void SetLocalPlayer(PlayerObject* pObject);
	PlayerObject *GetPlayer( int dpnid );
	PlayerObject *GetNextPlayer( bool restart = false );
	PlayerObject *GetViewingPlayer();

private:
	LinkedList< PlayerObject > *m_players; // Linked list of player objects.
	PlayerObject *m_viewingPlayer; // Pointer to the currently viewing player.

	bool m_localMovement; // Indicates if the local player moved in the current frame.
	float m_localDrive; // Local player's drive direction.
	float m_localStrafe; // Local player's strafe direction.
	bool m_localFire; // Indicates if the local player is firing their weapon.

	bool m_spawnLocalPlayer; // Indicates if the local player needs to be spawned.
	bool m_requestedSpawnPoint; // Indicates if the a spawn point has been requested.

	PlayerObject* m_pLocalPlayer;
};

#endif