//-----------------------------------------------------------------------------
// SpawnerObject.h implementation.
// Refer to the SpawnerObject.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "Engine.h"

//-----------------------------------------------------------------------------
// The spawner object class constructor.
//-----------------------------------------------------------------------------
SpawnerObject::SpawnerObject( char *name, char *path, unsigned long type ) : SceneObject( type )
{
	// Set spawner objects as ghosts.
	SetGhost( true );

	// Load the spawner's script.
	Script *script = new Script( name, path );

	// Get the spawner's frequency.
	m_frequency = *script->GetFloatData( "frequency" );

	// Clear the spawn timer.
	m_spawnTimer = 0.0f;

	// Load the sound to play when the spawner's object is collected.
	if( script->GetStringData( "sound" ) != NULL )
	{
		m_sound = new Sound( script->GetStringData( "sound" ) );
		m_audioPath = new AudioPath3D;
	}
	else
	{
		m_sound = NULL;
		m_audioPath = NULL;
	}

	// Load the script for the spawner's object.
	m_objectScript = g_engine->GetScriptManager()->Add( script->GetStringData( "object" ), script->GetStringData( "object_path" ) );

	// Get the name of the spawner's object.
	m_name = new char[strlen( m_objectScript->GetStringData( "name" ) ) + 1];
	strcpy( m_name, m_objectScript->GetStringData( "name" ) );

	// Set the spawner's mesh to use the object's mesh.
	SetMesh( m_objectScript->GetStringData( "mesh" ), m_objectScript->GetStringData( "mesh_path" ) );

	// Set the object to spin slowly.
	SetSpin( 0.0f, 1.0f, 0.0f );

	// Get the spawner's radius. A radius of 0.0 indicates that it must be
	// taken from the object to be spawned.
	if( *script->GetFloatData( "radius" ) != 0.0f )
		SetBoundingSphere( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), *script->GetFloatData( "radius" ) );
	else if( GetMesh() != NULL )
		SetEllipsoidRadius( *m_objectScript->GetVectorData( "ellipse_radius" ) );

	// Destroy the spawner's script.
	SAFE_DELETE( script );
}

//-----------------------------------------------------------------------------
// The spawner object class destructor.
//-----------------------------------------------------------------------------
SpawnerObject::~SpawnerObject()
{
	// Destroy the string buffer containing the name of the spawner's object.
	SAFE_DELETE_ARRAY( m_name );

	// Destroy the collect sound and its audio path.
	SAFE_DELETE( m_sound );
	SAFE_DELETE( m_audioPath );

	// Destroy the spawner's object script.
	g_engine->GetScriptManager()->Remove( &m_objectScript );
}

//-----------------------------------------------------------------------------
// Updates the spawner object.
//-----------------------------------------------------------------------------
void SpawnerObject::Update( float elapsed, bool addVelocity )
{
	// Allow the base scene object to update.
	SceneObject::Update( elapsed, addVelocity );

	// Check if the spawner is not visible
	if( GetVisible() == false )
	{
		// The spawner will become visible again after the set amount of
		// time (frequency) has passed.
		m_spawnTimer += elapsed;
		if( m_spawnTimer >= m_frequency )
		{
			SetVisible( true );
			SetIgnoreCollisions( false );
			m_spawnTimer = 0.0f;
		}
	}

	// Update the collect sound's audio path.
	if( m_audioPath != NULL )
	{
		m_audioPath->SetPosition( GetTranslation() );
		m_audioPath->SetVelocity( GetVelocity() );
	}
}

//-----------------------------------------------------------------------------
// Called when something collides with the spawner object.
//-----------------------------------------------------------------------------
void SpawnerObject::CollisionOccurred( SceneObject *object, unsigned long collisionStamp )
{
	// Allow the base scene object to register the collision.
	SceneObject::CollisionOccurred( object, collisionStamp );

	// Prevent the spawner's object from being collected by any of the default
	// engine objects. It can only be collected by user defined objects.
	if( object->GetType() == TYPE_SCENE_OBJECT || object->GetType() == TYPE_ANIMATED_OBJECT || object->GetType() == TYPE_SPAWNER_OBJECT )
		return;

	// Make the spawner invisible. This will prevent other objects from
	// colliding with it until it respawns (i.e. becomes visible again).
	SetVisible( false );
	SetIgnoreCollisions( true );

	// Play the collected sound.
	if( m_audioPath != NULL && m_sound != NULL )
		m_audioPath->Play( m_sound->GetSegment() );
}

//-----------------------------------------------------------------------------
// Returns the script of the spawned object.
//-----------------------------------------------------------------------------
Script *SpawnerObject::GetObjectScript()
{
	return m_objectScript;
}