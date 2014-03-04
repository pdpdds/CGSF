//-----------------------------------------------------------------------------
// Derived scene object to provide the functionality for a player.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef PLAYER_OBJECT_H
#define PLAYER_OBJECT_H

//-----------------------------------------------------------------------------
// Player Object Type Define
//-----------------------------------------------------------------------------
#define TYPE_PLAYER_OBJECT 3

//-----------------------------------------------------------------------------
// Animation Callback Data Structure
//-----------------------------------------------------------------------------
struct AnimationCallbackData
{
	char foot; // Identifies which foot caused the callback.
};

//-----------------------------------------------------------------------------
// Player Object Class
//-----------------------------------------------------------------------------
class PlayerObject : public AnimatedObject
{
public:
	PlayerObject( PlayerInfo *player, Script *script, unsigned long type = TYPE_PLAYER_OBJECT );
	virtual ~PlayerObject();

	virtual void Update( float elapsed, bool addVelocity = true );
	virtual void Render( D3DXMATRIX *world = NULL );

	virtual void CollisionOccurred( SceneObject *object, unsigned long collisionStamp );

	void MouseLook( float x, float y, bool reset = false );

	void Hurt( float damage, PlayerObject *attacker );
	void Kill();

	int GetID();
	char *GetName();

	void SetHealth( float health );
	float GetHealth();
	void SetDying( bool dying );
	bool GetDying();

	void SetIsViewing( bool isViewing );

	void SetFrags( unsigned long frags );
	unsigned long GetFrags();
	void SetDeaths( unsigned long deaths );
	unsigned long GetDeaths();

	void SetDrive( float drive );
	float GetDrive();
	void SetStrafe( float strafe );
	float GetStrafe();
	void SetFire( bool fire );
	bool GetFire();

	void ChangeWeapon( char change, char weapon = 0 );
	void WeaponChanging();
	void WeaponChanged( char weapon );
	void ClearWeapons();

	void SetViewTilt( float tilt );
	float GetViewTilt();
	D3DXVECTOR3 GetEyePoint();

private:
	virtual HRESULT CALLBACK HandleCallback( THIS_ UINT Track, LPVOID pCallbackData );

private:
	enum{ ANIM_IDLE, ANIM_DEATH, ANIM_FORWARDS, ANIM_BACKWARDS, ANIM_LEFT, ANIM_RIGHT };

	int m_dpnid; // DirectPlay ID of the player.
	char *m_name; // Name of the player.
	float m_health; // Player's health.
	bool m_dying; // Indicates if the player is dying.
	bool m_isViewing; // Inidicates if this player is the viewing player.

	unsigned long m_frags; // Player's frag count.
	unsigned long m_deaths; // Player's death tally.

	float m_drive; // Player's drive direction.
	float m_strafe; // Player's strafe direction.
	bool m_fire; // Indicates if the player is firing their weapon.

	D3DXVECTOR3 m_viewPoint; // Translation of the player's view point in model space.
	float m_viewTilt; // Player's view tilt (i.e. rotation around the x axis).

	float m_viewSmoothing; // Amount of smoothing applied to view movements (local player only).
	float m_viewSensitivity; // Sensitivity of view movements (local player only).

	Weapon *m_weapons[10]; // Array of weapons.
	char m_currentWeapon; // Current weapon.
	char m_oldWeapon; // Used for changing the player's weapon.
	D3DXVECTOR3 m_viewWeaponOffset; // Translation to apply to the attached weapon (3rd person view).
	float m_changingWeapon; // Timer used for changing the player's weapon.
	bool m_weaponChanging; // Indicates if the player is currently changing weapons.

	AnimationCallbackData m_callbackData[2]; // Data used for the foot step call back.
	RayIntersectionResult m_stepResult; // Used to determine if a foot is touching the ground.

	AudioPath3D *m_leftStepAudioPath; // Audio path for playing the left foot step sound.
	AudioPath3D *m_rightStepAudioPath; // Audio path for playing the right foot step sound.
};

#endif