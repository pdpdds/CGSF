//-----------------------------------------------------------------------------
// PlayerObject.h implementation.
// Refer to the PlayerObject.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "FPSMain.h"
#include "GamePacketStructure.h"
#include <SFPacketStore/PacketID.h>
#include <SFPacketStore/SFPacketStore.pb.h>
#include "CasualGameManager.h"
#include "NetworkSystem.h"
#include "SFProtobufPacket.h"
#include "BasePacket.h"

extern CasualGameManager* g_pCasualGameManager;

//-----------------------------------------------------------------------------
// The player object class constructor.
//-----------------------------------------------------------------------------
PlayerObject::PlayerObject( PlayerInfo *player, Script *script, unsigned long type ) : AnimatedObject( script->GetStringData( "mesh_name" ), script->GetStringData( "mesh_path" ), type )
{
	// Set the player's DirectPlay ID.
	m_dpnid = player->PlayerID;

	// Set the players's name.
	m_name = new char[strlen( "CGSF" ) + 1];
	strcpy( m_name, "CGSF");

	// Players start with full health;
	m_health = 100.0f;
	m_dying = false;

	// Indicate that the view transform is not being taken from this player.
	m_isViewing = false;

	// Clear the player's score.
	m_frags = 0;
	m_deaths = 0;

	// Player objects start off invisible and disabled.
	SetVisible( false );
	SetEnabled( false );

	// Clear the player's input.
	m_drive = 0.0f;
	m_strafe = 0.0f;
	m_fire = false;

	// Set the correct ellipse radius.
	SetEllipsoidRadius( *script->GetVectorData( "ellipse_radius" ) );

	// Level the player's view tilt.
	m_viewTilt = 0.0f;

	// Set the default view smoothing and sensitivity.
	m_viewSmoothing = 0.5f;
	m_viewSensitivity = 0.5f;

	// Get the view weapon offset.
	m_viewWeaponOffset = *script->GetVectorData( "view_weapon_offset" );

	// Clear the weapons array.
	for( unsigned char w = 0; w < 10; w++ )
		m_weapons[w] = NULL;

	// The player starts with the first basic weapon.
	Script *weaponScript = new Script( "Gun1.txt", "./Assets/Objects/Gun1/" );
	m_weapons[0] = new Weapon( weaponScript, m_viewWeaponOffset );
	m_currentWeapon = 0;
	//if( player->dpnid == g_engine->GetNetwork()->GetPlayerID() )
	//	m_weapons[m_currentWeapon]->UseViewWeapon( true );
	//else
	m_weapons[m_currentWeapon]->UseViewWeapon( false );
	SAFE_DELETE( weaponScript );

	// Indicate that the player is not changing weapons.
	m_changingWeapon = 0.0f;
	m_weaponChanging = false;

	// Create the callback data used for tracking the player's foot steps.
	m_callbackData[0].foot = 0;
	m_callbackData[1].foot = 1;

	// Create the callback keys. The second key time is set per animation.
	D3DXKEY_CALLBACK keys[2];
	keys[0].Time = 0;
	keys[0].pCallbackData = &m_callbackData[0];
	keys[1].pCallbackData = &m_callbackData[1];

	LPD3DXKEYFRAMEDANIMATIONSET oldAS;
	LPD3DXCOMPRESSEDANIMATIONSET newAS;
	LPD3DXBUFFER buffer;

	// Go through the four movement animations and set the foot step keys.
	for( char a = 1; a < 5; a++ )
	{
		// Get the old animation.
		GetAnimationController()->GetAnimationSet( a, (LPD3DXANIMATIONSET*)&oldAS );

		// Set the time for the second key.
		keys[1].Time = float( oldAS->GetPeriod() / 2.0f * oldAS->GetSourceTicksPerSecond() );

		// Compress the old animation set.
		oldAS->Compress( D3DXCOMPRESS_DEFAULT, 0.4f, NULL, &buffer );

		// Create the new animation using the old one and the foot step keys.
		D3DXCreateCompressedAnimationSet( oldAS->GetName(), oldAS->GetSourceTicksPerSecond(), oldAS->GetPlaybackType(), buffer, 2, keys, &newAS );
		SAFE_RELEASE( buffer );

		// Unregister the old animation set.
		GetAnimationController()->UnregisterAnimationSet( oldAS );
		SAFE_RELEASE( oldAS );

		// Register the new animation set.
		// Note: The new animation is appended to the end of the list.
		GetAnimationController()->RegisterAnimationSet( newAS );
		SAFE_RELEASE( newAS );
	}

	// Play the idle animation.
	PlayAnimation( 0, 0.0f );

	// Create the step sound audio paths.
//	m_leftStepAudioPath = new AudioPath3D;
//	m_rightStepAudioPath = new AudioPath3D;

	// Set the friction on this object.
	SetFriction( 8.0f );
}

//-----------------------------------------------------------------------------
// The player object class destructor.
//-----------------------------------------------------------------------------
PlayerObject::~PlayerObject()
{
	// Destroy the weapons array.
	for( unsigned char w = 0; w < 10; w++ )
		SAFE_DELETE( m_weapons[w] );

	// Destroy the string buffer containing the player's name.
	SAFE_DELETE( m_name );
}

//-----------------------------------------------------------------------------
// Updates the player object.
//-----------------------------------------------------------------------------
void PlayerObject::Update( float elapsed, bool addVelocity )
{
	// Allow the base animated object to update.
	AnimatedObject::Update( elapsed, addVelocity );

	// Override the object's forward vector to take the view tilt into account.
	// This will allow the forward vector to move up and down as well instead
	// of just remaining horizontal. This is not important for movement since
	// the player can not fly, but for things like shooting it is.
	m_forward.x = (float)sin( GetRotation().y );
	m_forward.y = (float)-tan( m_viewTilt );
	m_forward.z = (float)cos( GetRotation().y );
	D3DXVec3Normalize( &m_forward, &m_forward );

	// Set the player's view point. This is done every frame because as the
	// mesh is animated, the reference point in the mesh may move. This
	// will allow the view point to move with the mesh's animations.
	m_viewPoint = GetMesh()->GetReferencePoint( "rp_view_point" )->GetTranslation();

	// Ensure that the view movement is relative to the rotation.
	D3DXVec3TransformCoord( &m_viewPoint, &m_viewPoint, GetRotationMatrix() );

	// Only calculate the correct view matrix if it is being used.
	if( m_isViewing == true )
	{
		// Create the x axis rotation matrix.
		D3DXMATRIX rotationXMatrix;
		D3DXMatrixRotationX( &rotationXMatrix, m_viewTilt );

		// Create the combined rotation matrix (i.e. y axis rotation from the
		// scene object plus the x axis rotation from the player object).
		D3DXMATRIX combinedRotation;
		D3DXMatrixMultiply( &combinedRotation, &rotationXMatrix, GetRotationMatrix() );

		// Build a translation matrix that represents the final view point.
		D3DXMATRIX viewPointTranslationMatrix;
		D3DXVECTOR3 finalViewPointTranslation = GetTranslation() + m_viewPoint;
		D3DXMatrixTranslation( &viewPointTranslationMatrix, finalViewPointTranslation.x, finalViewPointTranslation.y, finalViewPointTranslation.z );

		// Override the object's view matrix using the combined rotation and
		// the position of the final view point translation.
		D3DXMatrixMultiply( &m_viewMatrix, &combinedRotation, &viewPointTranslationMatrix );
		D3DXMatrixInverse( &m_viewMatrix, NULL, &m_viewMatrix );
	}

	// Ignore the rest if the player is dying (or dead)
	if( m_dying == true )
		return;

	// Drive and strafe the player accordingly.
	if( m_drive != 0.0f )
		Drive( m_drive * 8000.0f * elapsed );
	if( m_strafe != 0.0f )
		Strafe( m_strafe * 4000.0f * elapsed );

	// Update the step audio paths.
//	m_leftStepAudioPath->SetPosition( GetTranslation() + GetMesh()->GetReferencePoint( "rp_left_foot" )->GetTranslation() );
//	m_leftStepAudioPath->SetVelocity( GetVelocity() );
//	m_rightStepAudioPath->SetPosition( GetTranslation() + GetMesh()->GetReferencePoint( "rp_right_foot" )->GetTranslation() );
//	m_rightStepAudioPath->SetVelocity( GetVelocity() );

	// Check if the player is changing their weapon (local player only).
	static float move = 0.0f;
	if( m_changingWeapon > 0.0f )
	{
		m_changingWeapon -= elapsed;

		if( m_changingWeapon > 1.0f )
		{
			// Lower the old weapon.
			move -= 100.0f * elapsed;
			m_weapons[m_oldWeapon]->RaiseLowerWeapon( elapsed, this, move );
		}
		else if( m_changingWeapon < 0.0f )
		{
			// The new weapon is in place.
			m_changingWeapon = 0.0f;
			move = 0.0f;

			// Send a message to inform the other players the weapon is ready.
			PlayerWeaponChangeMsg pwcm;
			pwcm.msgid = CGSF::MSG_PLAYER_WEAPON_CHANGE;
			pwcm.PlayerID = g_engine->GetPlayerID();
			pwcm.weapon = m_currentWeapon;
			
			SFProtobufPacket<SFPacketStore::MSG_PLAYER_WEAPON_CHANGE> request(CGSF::MSG_PLAYER_WEAPON_CHANGE);
			request.GetData().set_weaponchange(&pwcm, sizeof(PlayerWeaponChangeMsg));
			g_pCasualGameManager->GetNetwork()->TCPSend(&request);
		}
		else
		{
			// Raise the new weapon.
			move += 100.0f * elapsed;
			m_weapons[m_currentWeapon]->RaiseLowerWeapon( elapsed, this, move );
		}
	}
	else
	{
		// Update the player's current weapon, when not changing weapons.
		if( m_weaponChanging == false )
			m_weapons[m_currentWeapon]->Update( elapsed, m_fire, this, m_viewPoint.y );
	}
}

//-----------------------------------------------------------------------------
// Renders the player object.
//-----------------------------------------------------------------------------
void PlayerObject::Render( D3DXMATRIX *world )
{
	// Allow the base animated object to render.
	if( m_dpnid != g_engine->GetPlayerID() )
		AnimatedObject::Render( world );
	else if( m_dying == true )
		return;

	// Render the player's weapon.
	if( m_weaponChanging == false )
	{
		if( m_changingWeapon > 1.0f )
			m_weapons[m_oldWeapon]->Render( this );
		else
			m_weapons[m_currentWeapon]->Render( this );
	}
}

//-----------------------------------------------------------------------------
// Called when something collides with the object.
//-----------------------------------------------------------------------------
void PlayerObject::CollisionOccurred( SceneObject *object, unsigned long collisionStamp )
{
	// Ignore collisions if the player is dying (or dead)
	if( m_dying == true )
		return;

	// Allow the base scene object to register the collision.
	SceneObject::CollisionOccurred( object, collisionStamp );

	// Check if the player has hit a spawner object.
	if( object->GetType() != TYPE_SPAWNER_OBJECT )
		return;

	// Get a pointer to the spawner object.
	SpawnerObject *spawner = (SpawnerObject*)object;

	// Check if the player has picked up a weapon.
	if( *spawner->GetObjectScript()->GetNumberData( "type" ) == WEAPON_SPAWN_OBJECT )
	{
		// Get the list position of the weapon.
		char listPosition = (char)*spawner->GetObjectScript()->GetNumberData( "list_position" );

		// Ensure the player doesn't already have a weapon in this slot.
		if( m_weapons[listPosition] == NULL )
		{
			// Load the new weapon in.
			m_weapons[listPosition] = new Weapon( spawner->GetObjectScript(), m_viewWeaponOffset );

			// Check if this is the local player.
			if( m_dpnid == g_engine->GetPlayerID() )
			{
				// Set the weapon to use the first person mesh.
				m_weapons[listPosition]->UseViewWeapon( true );

				// Change to this weapon.
				ChangeWeapon( 0, listPosition );
			}
			else
			{
				// Set the weapon to use the first person mesh.
				m_weapons[listPosition]->UseViewWeapon( false );
			}
		}
		else if( m_weapons[listPosition]->GetValid() == false )
		{
			// Validate the weapon.
			m_weapons[listPosition]->SetValid( true );

			// Change to this weapon.
			ChangeWeapon( 0, listPosition );
		}
	}
}

//-----------------------------------------------------------------------------
// Rotates the player's view.
//-----------------------------------------------------------------------------
void PlayerObject::MouseLook( float x, float y, bool reset )
{
	static float lastX = 0.0f;
	static float lastY = 0.0f;

	// Check if the player's view needs to be reset.
	if( reset == true )
	{
		lastX = lastY = 0.0f;
		SetRotation( 0.0f, 0.0f, 0.0f );
		m_viewTilt = 0.0f;
		return;
	}

	// Calculate the real x and y values by accounting for smoothing.
	lastX = lastX * m_viewSmoothing + x * ( 1.0f - m_viewSmoothing );
	lastY = lastY * m_viewSmoothing + y * ( 1.0f - m_viewSmoothing );

	// Adjust the values for sensitivity.
	lastX *= m_viewSensitivity;
	lastY *= m_viewSensitivity;

	// Rotate the scene object around the y axis only. This will prevent the
	// player's mesh from rotating when the player looks up and down.
	AddRotation( 0.0f, lastY, 0.0f );

	// Ensure the view will not rotate to far up or down.
	if( ( m_viewTilt > 0.8f && lastX > 0.0f ) || ( m_viewTilt < -0.8f && lastX < 0.0f ) )
		lastX = 0.0f;

	// Maintain a seperate view rotation around the x axis to allow the player
	// to look up and down.
	m_viewTilt += lastX;
}

//-----------------------------------------------------------------------------
// Hurts the player with the given damage.
//-----------------------------------------------------------------------------
void PlayerObject::Hurt( float damage, PlayerObject *attacker )
{
	// Ignore dying (or dead) players.
	if( m_dying == true )
		return;

	// Adjust the player's health.
	m_health -= damage;

	// Send a player health update message.
	PlayerHealthMsg phm;
	phm.msgid = CGSF::MSG_PLAYER_HEALTH;
	phm.PlayerID = m_dpnid;
	phm.health = m_health;
	
	SFProtobufPacket<SFPacketStore::MSG_PLAYER_HEALTH> request(CGSF::MSG_PLAYER_HEALTH);
	request.GetData().set_playerhealth(&phm, sizeof(PlayerHealthMsg));
	g_pCasualGameManager->GetNetwork()->TCPSend(&request);

	// Check if the player is still alive (i.e. health above zero).
	/*if( m_health > 0.0f )
		return;

	// The player has been killed.
	Kill();

	// Increment the player's death tally.
	m_deaths += 1;

	// Send a player score update message for the fragged player.
	PlayerScoreMsg psm1;
	psm1.msgid = CGSF::MSG_PLAYER_SCORE;
	//psm1.dpnid = m_dpnid;
	psm1.frags = m_frags;
	psm1.deaths = m_deaths;
	//g_engine->GetNetwork()->Send( &psm1, sizeof( PlayerScoreMsg ), DPNID_ALL_PLAYERS_GROUP, DPNSEND_NOLOOPBACK );

	// Increment the attacking player's frag tally.
	attacker->SetFrags( attacker->GetFrags() + 1 );

	// Send a player score update message for the bullet owner.
	PlayerScoreMsg psm2;
	psm2.msgid = CGSF::MSG_PLAYER_SCORE;
	//psm2.dpnid = attacker->GetID();
	psm2.frags = attacker->GetFrags();
	psm2.deaths = attacker->GetDeaths();
	//g_engine->GetNetwork()->Send( &psm2, sizeof( PlayerScoreMsg ), DPNID_ALL_PLAYERS_GROUP, DPNSEND_NOLOOPBACK );
	*/
}

//-----------------------------------------------------------------------------
// Kills the player.
//-----------------------------------------------------------------------------
void PlayerObject::Kill()
{
	// Indicate that the player is dying.
	m_dying = true;

	// Clear the player's movment.
	SetDrive( 0.0f );
	SetStrafe( 0.0f );
	SetFire( false );
	Stop();

	// Clear the player's weapons.
	ClearWeapons();

	// Stop the player from changing weapons.
	m_changingWeapon = 0.0f;
	m_weaponChanging = false;

	// Play the death animation.
	PlayAnimation( ANIM_DEATH, 0.0f, false );
}

//-----------------------------------------------------------------------------
// Returns the player object's DirectPlay ID number.
//-----------------------------------------------------------------------------
int PlayerObject::GetID()
{
	return m_dpnid;
}

//-----------------------------------------------------------------------------
// Returns the player object's name.
//-----------------------------------------------------------------------------
char *PlayerObject::GetName()
{
	return m_name;
}

//-----------------------------------------------------------------------------
// Sets the player object's health.
//-----------------------------------------------------------------------------
void PlayerObject::SetHealth( float health )
{
	m_health = health;
}

//-----------------------------------------------------------------------------
// Returns the player object's health.
//-----------------------------------------------------------------------------
float PlayerObject::GetHealth()
{
	return m_health;
}

//-----------------------------------------------------------------------------
// Sets the player object's dying flag.
//-----------------------------------------------------------------------------
void PlayerObject::SetDying( bool dying )
{
	m_dying = dying;
}

//-----------------------------------------------------------------------------
// Returns the player object's dying flag.
//-----------------------------------------------------------------------------
bool PlayerObject::GetDying()
{
	return m_dying;
}

//-----------------------------------------------------------------------------
// Sets the player object's viewing flag.
//-----------------------------------------------------------------------------
void PlayerObject::SetIsViewing( bool isViewing )
{
	m_isViewing = isViewing;
}

//-----------------------------------------------------------------------------
// Sets the player object's frag count.
//-----------------------------------------------------------------------------
void PlayerObject::SetFrags( unsigned long frags )
{
	m_frags = frags;
}

//-----------------------------------------------------------------------------
// Returns the player object's frag count.
//-----------------------------------------------------------------------------
unsigned long PlayerObject::GetFrags()
{
	return m_frags;
}

//-----------------------------------------------------------------------------
// Sets the player object's death count.
//-----------------------------------------------------------------------------
void PlayerObject::SetDeaths( unsigned long deaths )
{
	m_deaths = deaths;
}

//-----------------------------------------------------------------------------
// Returns the player object's death count.
//-----------------------------------------------------------------------------
unsigned long PlayerObject::GetDeaths()
{
	return m_deaths;
}

//-----------------------------------------------------------------------------
// Sets the player object's drive.
//-----------------------------------------------------------------------------
void PlayerObject::SetDrive( float drive )
{
	// Ignore if the player is dying (or dead)
	if( m_dying == true )
		return;

	if( drive == 0.0f )
	{
		if( m_drive != 0.0f && m_strafe == 0.0f )
			PlayAnimation( ANIM_IDLE, 0.2f );
	}
	else if( m_drive == 0.0f && m_strafe == 0.0f )
	{
		if( drive == 1.0f )
			PlayAnimation( ANIM_FORWARDS, 0.2f );
		else
			PlayAnimation( ANIM_BACKWARDS, 0.2f );
	}

	m_drive = drive;
}

//-----------------------------------------------------------------------------
// Returns the player object's current drive.
//-----------------------------------------------------------------------------
float PlayerObject::GetDrive()
{
	return m_drive;
}

//-----------------------------------------------------------------------------
// Sets the player object's strafe.
//-----------------------------------------------------------------------------
void PlayerObject::SetStrafe( float strafe )
{
	// Ignore if the player is dying (or dead)
	if( m_dying == true )
		return;

	if( strafe == 0.0f )
	{
		if( m_strafe != 0.0f && m_drive == 0.0f )
			PlayAnimation( ANIM_IDLE, 0.2f );
	}
	else if( m_strafe == 0.0f && m_drive == 0.0f )
	{
		if( strafe == 1.0f )
			PlayAnimation( ANIM_RIGHT, 0.2f );
		else
			PlayAnimation( ANIM_LEFT, 0.2f );
	}

	m_strafe = strafe;
}

//-----------------------------------------------------------------------------
// Returns the player object's current strafe.
//-----------------------------------------------------------------------------
float PlayerObject::GetStrafe()
{
	return m_strafe;
}

//-----------------------------------------------------------------------------
// Sets the player object's fire flag.
//-----------------------------------------------------------------------------
void PlayerObject::SetFire( bool fire )
{
	// Ignore if the player is dying (or dead)
	if( m_dying == true )
		return;

	m_fire = fire;
}

//-----------------------------------------------------------------------------
// Returns the player object's fire flag.
//-----------------------------------------------------------------------------
bool PlayerObject::GetFire()
{
	return m_fire;
}

//-----------------------------------------------------------------------------
// Changes weapon in either the given direction or to the given weapon.
//-----------------------------------------------------------------------------
void PlayerObject::ChangeWeapon( char change, char weapon )
{
	// Ignore if the player is dying (or dead)
	if( m_dying == true )
		return;

	// If the player is already changing weapons, then ingore this request.
	if( m_changingWeapon > 0.0f )
		return;

	// Store the player's current weapon.
	m_oldWeapon = m_currentWeapon;

	// Check if the player is changing to a specific weapon.
	if( change == 0 )
	{
		// Change to the weapon and ensure it is valid.
		m_currentWeapon = weapon;
		if( m_weapons[m_currentWeapon] == NULL )
			m_currentWeapon = m_oldWeapon;
		else if( m_weapons[m_currentWeapon]->GetValid() == false )
			m_currentWeapon = m_oldWeapon;
	}
	else
	{
		// Loop through the weapons until the next valid weapon is found.
		do
		{
			do
			{
				m_currentWeapon += change;

				if( m_currentWeapon < 0 )
					m_currentWeapon = 9;
				else if( m_currentWeapon > 9 )
					m_currentWeapon = 0;
			}
			while( m_weapons[m_currentWeapon] == NULL );
		}
		while( m_weapons[m_currentWeapon]->GetValid() == false );
	}

	// Ensure the player actually changed to a different weapon.
	if( m_oldWeapon == m_currentWeapon )
		return;

	// Begin the weapon change process.
	m_changingWeapon = 2.0f;

	// Send a weapon changing message.
	NetworkMessage pwcm;
	pwcm.msgid = CGSF::MSG_PLAYER_WEAPON_CHANGING;
	pwcm.PlayerID = g_engine->GetPlayerID();

	SFProtobufPacket<SFPacketStore::MSG_PLAYER_WEAPON_CHANGING> request(CGSF::MSG_PLAYER_WEAPON_CHANGING);
	request.GetData().set_weaponchanging(&pwcm, sizeof(NetworkMessage));
	g_pCasualGameManager->GetNetwork()->TCPSend(&request);
}

//-----------------------------------------------------------------------------
// Indicates that this player is currently changing weapons.
//-----------------------------------------------------------------------------
void PlayerObject::WeaponChanging()
{
	m_weaponChanging = true;
}

//-----------------------------------------------------------------------------
// Indicates that this player has changed weapons to the given weapon.
//-----------------------------------------------------------------------------
void PlayerObject::WeaponChanged( char weapon )
{
	m_currentWeapon = weapon;
	m_weaponChanging = false;
}

//-----------------------------------------------------------------------------
// Clears all the player object's weapons, except the first one.
//-----------------------------------------------------------------------------
void PlayerObject::ClearWeapons()
{
	// Invalidate all but the player's first weapon.
	for( unsigned char w = 1; w < 10; w++ )
		if( m_weapons[w] != NULL )
			m_weapons[w]->SetValid( false );

	// Set the current weapon and weapon changing flags.
	m_currentWeapon = 0;
	m_changingWeapon = 0.0f;
	m_weaponChanging = false;
}

//-----------------------------------------------------------------------------
// Sets the player object's view tilt (i.e. the rotation around the x axis).
//-----------------------------------------------------------------------------
void PlayerObject::SetViewTilt( float tilt )
{
	m_viewTilt = tilt;
}

//-----------------------------------------------------------------------------
// Returns the player object's view tilt (i.e. the rotation around the x axis).
//-----------------------------------------------------------------------------
float PlayerObject::GetViewTilt()
{
	return m_viewTilt;
}

//-----------------------------------------------------------------------------
// Returns the player object's eye point.
//-----------------------------------------------------------------------------
D3DXVECTOR3 PlayerObject::GetEyePoint()
{
	return GetTranslation() + m_viewPoint;
}

//-----------------------------------------------------------------------------
// Animation call back handler.
//-----------------------------------------------------------------------------
HRESULT CALLBACK PlayerObject::HandleCallback( THIS_ UINT Track, LPVOID pCallbackData )
{
	// Get a pointer to the callback data.
	AnimationCallbackData *data = (AnimationCallbackData*)pCallbackData;

	// If the player is not touching the ground, then it can't make foot steps.
	if( IsTouchingGround() == false )
		return S_OK;

	// Check which foot caused the callback.
	if( data->foot == 1 )
	{
		// Reset the step result.
		m_stepResult.material = NULL;

		// Preform a ray intersection for the left foot. If it is succesful,
		// then play the material's step sound.
		if( g_engine->GetSceneManager()->RayIntersectScene( &m_stepResult, GetTranslation() + GetMesh()->GetReferencePoint( "rp_left_foot" )->GetTranslation(), D3DXVECTOR3( 0.0f, -1.0f, 0.0f ) ) == true )
		{
			//if( ( (GameMaterial*)m_stepResult.material )->GetStepSound() != NULL )
			//	m_leftStepAudioPath->Play( ( (GameMaterial*)m_stepResult.material )->GetStepSound()->GetSegment() );
		}
	}
	else
	{
		// Reset the step result.
		m_stepResult.material = NULL;

		// Preform a ray intersection for the right foot. If it is succesful,
		// then play the material's step sound.
		if( g_engine->GetSceneManager()->RayIntersectScene( &m_stepResult, GetTranslation() + GetMesh()->GetReferencePoint( "rp_right_foot" )->GetTranslation(), D3DXVECTOR3( 0.0f, -1.0f, 0.0f ) ) == true )
		{
			//if( ( (GameMaterial*)m_stepResult.material )->GetStepSound() != NULL )
			//	m_rightStepAudioPath->Play( ( (GameMaterial*)m_stepResult.material )->GetStepSound()->GetSegment() );
		}
	}

	return S_OK;
}