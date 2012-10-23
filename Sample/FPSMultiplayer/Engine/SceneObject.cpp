//-----------------------------------------------------------------------------
// SceneObject.h implementation.
// Refer to the SceneObject.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "Engine.h"

//-----------------------------------------------------------------------------
// The scene object class constructor.
//-----------------------------------------------------------------------------
SceneObject::SceneObject( unsigned long type, char *meshName, char *meshPath, bool sharedMesh )
{
	// Set the object's type;
	SetType( type );

	// Zero the scene object's translation and rotation.
	SetTranslation( 0.0f, 0.0f, 0.0f );
	SetRotation( 0.0f, 0.0f, 0.0f );

	// The object is initially at rest.
	SetVelocity( 0.0f, 0.0f, 0.0f );
	SetSpin( 0.0f, 0.0f, 0.0f );

	// The object is initially facing into the positive z-axis.
	m_forward = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
	m_right = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );

	// Initially the object has no friction.
	m_friction = 0.0f;

	// Clear the collision stamp.
	m_collisionStamp = -1;

	// The object is visible, enabled, solid, and registering collisons by default.
	m_visible = true;
	m_enabled = true;
	m_ghost = false;
	m_ignoreCollisions = false;

	// Initially the object is not touching the ground.
	m_touchingGround = false;

	// Set the object's mesh.
	m_mesh = NULL;
	SetMesh( meshName, meshPath, sharedMesh );
}

//-----------------------------------------------------------------------------
// The scene object class destructor.
//-----------------------------------------------------------------------------
SceneObject::~SceneObject()
{
	// Destroy the object's mesh.
	if( m_sharedMesh == true )
		g_engine->GetMeshManager()->Remove( &m_mesh );
	else
		SAFE_DELETE( m_mesh );
}

//-----------------------------------------------------------------------------
// Updates the object.
//-----------------------------------------------------------------------------
void SceneObject::Update( float elapsed, bool addVelocity )
{
	// Calculate the friction for this update.
	float friction = 1.0f - m_friction * elapsed;

	// Move the object.
	m_velocity *= friction;
	if( addVelocity == true )
	{
		D3DXVECTOR3 velocity = m_velocity * elapsed;
		AddTranslation( velocity.x, velocity.y, velocity.z );
	}

	// Spin the object.
	m_spin *= friction;
	D3DXVECTOR3 spin = m_spin * elapsed;
	AddRotation( spin.x, spin.y, spin.z );

	// Update the object's world matrix.
	D3DXMatrixMultiply( &m_worldMatrix, &m_rotationMatrix, &m_translationMatrix );

	// Create a view matrix for the object.
	D3DXMatrixInverse( &m_viewMatrix, NULL, &m_worldMatrix );

	// Update the object's forward vector.
	m_forward.x = (float)sin( m_rotation.y );
	m_forward.y = (float)-tan( m_rotation.x );
	m_forward.z = (float)cos( m_rotation.y );
	D3DXVec3Normalize( &m_forward, &m_forward );

	// Update the object's right vector.
	m_right.x = (float)cos( m_rotation.y );
	m_right.y = (float)tan( m_rotation.z );
	m_right.z = (float)-sin( m_rotation.y );
	D3DXVec3Normalize( &m_right, &m_right );

	// Update the object's bounding volume using the translation matrix only.
	// This will maintain an axis aligned bounding box around the object in
	// world space rather than the object's local space.
	RepositionBoundingVolume( &m_translationMatrix );
}

//-----------------------------------------------------------------------------
// Renders the object.
//-----------------------------------------------------------------------------
void SceneObject::Render( D3DXMATRIX *world )
{
	// Ignore the object if it has no mesh.
	if( m_mesh == NULL )
		return;

	// Check if the object's world tranformation matrix has been overridden.
	if( world == NULL )
		g_engine->GetDevice()->SetTransform( D3DTS_WORLD, &m_worldMatrix );
	else
		g_engine->GetDevice()->SetTransform( D3DTS_WORLD, world );

	// Render the object's mesh.
	m_mesh->Render();
}

//-----------------------------------------------------------------------------
// Called when something collides with the object.
//-----------------------------------------------------------------------------
void SceneObject::CollisionOccurred( SceneObject *object, unsigned long collisionStamp )
{
	// Set the collision stamp.
	m_collisionStamp = collisionStamp;
}

//-----------------------------------------------------------------------------
// Applies the given force to the object in the forwards/backwards direction.
//-----------------------------------------------------------------------------
void SceneObject::Drive( float force, bool lockYAxis )
{
	D3DXVECTOR3 realForce = m_forward * force;

	m_velocity.x += realForce.x;
	m_velocity.z += realForce.z;

	if( lockYAxis == false )
		m_velocity.y += realForce.y;
}

//-----------------------------------------------------------------------------
// Applies the given force to the object in the right/left direction.
//-----------------------------------------------------------------------------
void SceneObject::Strafe( float force, bool lockYAxis )
{
	D3DXVECTOR3 realForce = m_right * force;

	m_velocity.x += realForce.x;
	m_velocity.z += realForce.z;

	if( lockYAxis == false )
		m_velocity.y += realForce.y;
}

//-----------------------------------------------------------------------------
// Stops the object moving.
//-----------------------------------------------------------------------------
void SceneObject::Stop()
{
	m_velocity = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_spin = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
}

//-----------------------------------------------------------------------------
// Sets the object's translation.
//-----------------------------------------------------------------------------
void SceneObject::SetTranslation( float x, float y, float z )
{
	m_translation.x = x;
	m_translation.y = y;
	m_translation.z = z;

	D3DXMatrixTranslation( &m_translationMatrix, m_translation.x, m_translation.y, m_translation.z );
}

//-----------------------------------------------------------------------------
// Sets the object's translation.
//-----------------------------------------------------------------------------
void SceneObject::SetTranslation( D3DXVECTOR3 translation )
{
	m_translation = translation;

	D3DXMatrixTranslation( &m_translationMatrix, m_translation.x, m_translation.y, m_translation.z );
}

//-----------------------------------------------------------------------------
// Adds the given translation to the object's current translation.
//-----------------------------------------------------------------------------
void SceneObject::AddTranslation( float x, float y, float z )
{
	m_translation.x += x;
	m_translation.y += y;
	m_translation.z += z;

	D3DXMatrixTranslation( &m_translationMatrix, m_translation.x, m_translation.y, m_translation.z );
}

//-----------------------------------------------------------------------------
// Adds the given translation to the object's current translation.
//-----------------------------------------------------------------------------
void SceneObject::AddTranslation( D3DXVECTOR3 translation )
{
	m_translation += translation;

	D3DXMatrixTranslation( &m_translationMatrix, m_translation.x, m_translation.y, m_translation.z );
}

//-----------------------------------------------------------------------------
// Returns the object's translation.
//-----------------------------------------------------------------------------
D3DXVECTOR3 SceneObject::GetTranslation()
{
	return m_translation;
}

//-----------------------------------------------------------------------------
// Sets the object's rotation.
//-----------------------------------------------------------------------------
void SceneObject::SetRotation( float x, float y, float z )
{
	m_rotation.x = x;
	m_rotation.y = y;
	m_rotation.z = z;

	D3DXMATRIX rotationX, rotationY;
	D3DXMatrixRotationX( &rotationX, m_rotation.x );
	D3DXMatrixRotationY( &rotationY, m_rotation.y );
	D3DXMatrixRotationZ( &m_rotationMatrix, m_rotation.z );
	D3DXMatrixMultiply( &m_rotationMatrix, &m_rotationMatrix, &rotationX );
	D3DXMatrixMultiply( &m_rotationMatrix, &m_rotationMatrix, &rotationY );
}

//-----------------------------------------------------------------------------
// Sets the object's rotation.
//-----------------------------------------------------------------------------
void SceneObject::SetRotation( D3DXVECTOR3 rotation )
{
	m_rotation = rotation;

	D3DXMATRIX rotationX, rotationY;
	D3DXMatrixRotationX( &rotationX, m_rotation.x );
	D3DXMatrixRotationY( &rotationY, m_rotation.y );
	D3DXMatrixRotationZ( &m_rotationMatrix, m_rotation.z );
	D3DXMatrixMultiply( &m_rotationMatrix, &m_rotationMatrix, &rotationX );
	D3DXMatrixMultiply( &m_rotationMatrix, &m_rotationMatrix, &rotationY );
}

//-----------------------------------------------------------------------------
// Adds the given rotation to the object's current rotation.
//-----------------------------------------------------------------------------
void SceneObject::AddRotation( float x, float y, float z )
{
	m_rotation.x += x;
	m_rotation.y += y;
	m_rotation.z += z;

	D3DXMATRIX rotationX, rotationY;
	D3DXMatrixRotationX( &rotationX, m_rotation.x );
	D3DXMatrixRotationY( &rotationY, m_rotation.y );
	D3DXMatrixRotationZ( &m_rotationMatrix, m_rotation.z );
	D3DXMatrixMultiply( &m_rotationMatrix, &m_rotationMatrix, &rotationX );
	D3DXMatrixMultiply( &m_rotationMatrix, &m_rotationMatrix, &rotationY );
}

//-----------------------------------------------------------------------------
// Adds the given rotation to the object's current rotation.
//-----------------------------------------------------------------------------
void SceneObject::AddRotation( D3DXVECTOR3 rotation )
{
	m_rotation += rotation;

	D3DXMATRIX rotationX, rotationY;
	D3DXMatrixRotationX( &rotationX, m_rotation.x );
	D3DXMatrixRotationY( &rotationY, m_rotation.y );
	D3DXMatrixRotationZ( &m_rotationMatrix, m_rotation.z );
	D3DXMatrixMultiply( &m_rotationMatrix, &m_rotationMatrix, &rotationX );
	D3DXMatrixMultiply( &m_rotationMatrix, &m_rotationMatrix, &rotationY );
}

//-----------------------------------------------------------------------------
// Returns the object's rotation.
//-----------------------------------------------------------------------------
D3DXVECTOR3 SceneObject::GetRotation()
{
	return m_rotation;
}

//-----------------------------------------------------------------------------
// Sets the object's velocity.
//-----------------------------------------------------------------------------
void SceneObject::SetVelocity( float x, float y, float z )
{
	m_velocity.x = x;
	m_velocity.y = y;
	m_velocity.z = z;
}

//-----------------------------------------------------------------------------
// Sets the object's velocity.
//-----------------------------------------------------------------------------
void SceneObject::SetVelocity( D3DXVECTOR3 velocity )
{
	m_velocity = velocity;
}

//-----------------------------------------------------------------------------
// Adds the given velocity to the object's current velocity.
//-----------------------------------------------------------------------------
void SceneObject::AddVelocity( float x, float y, float z )
{
	m_velocity.x += x;
	m_velocity.y += y;
	m_velocity.z += z;
}

//-----------------------------------------------------------------------------
// Adds the given velocity to the object's current velocity.
//-----------------------------------------------------------------------------
void SceneObject::AddVelocity( D3DXVECTOR3 velocity )
{
	m_velocity += velocity;
}

//-----------------------------------------------------------------------------
// Returns the object's velocity.
//-----------------------------------------------------------------------------
D3DXVECTOR3 SceneObject::GetVelocity()
{
	return m_velocity;
}

//-----------------------------------------------------------------------------
// Sets the object's spin.
//-----------------------------------------------------------------------------
void SceneObject::SetSpin( float x, float y, float z )
{
	m_spin.x = x;
	m_spin.y = y;
	m_spin.z = z;
}

//-----------------------------------------------------------------------------
// Sets the object's spin.
//-----------------------------------------------------------------------------
void SceneObject::SetSpin( D3DXVECTOR3 spin )
{
	m_spin = spin;
}

//-----------------------------------------------------------------------------
// Adds the given spin to the object's current spin.
//-----------------------------------------------------------------------------
void SceneObject::AddSpin( float x, float y, float z )
{
	m_spin.x += x;
	m_spin.y += y;
	m_spin.z += z;
}

//-----------------------------------------------------------------------------
// Adds the given spin to the object's current spin.
//-----------------------------------------------------------------------------
void SceneObject::AddSpin( D3DXVECTOR3 spin )
{
	m_spin = spin;
}

//-----------------------------------------------------------------------------
// Returns the object's spin.
//-----------------------------------------------------------------------------
D3DXVECTOR3 SceneObject::GetSpin()
{
	return m_spin;
}

//-----------------------------------------------------------------------------
// Returns the object's forward vector.
//-----------------------------------------------------------------------------
D3DXVECTOR3 SceneObject::GetForwardVector()
{
	return m_forward;
}

//-----------------------------------------------------------------------------
// Returns the object's right vector.
//-----------------------------------------------------------------------------
D3DXVECTOR3 SceneObject::GetRightVector()
{
	return m_right;
}

//-----------------------------------------------------------------------------
// Returns a pointer to the object's current translation matrix.
//-----------------------------------------------------------------------------
D3DXMATRIX *SceneObject::GetTranslationMatrix()
{
	return &m_translationMatrix;
}

//-----------------------------------------------------------------------------
// Returns a pointer to the object's current rotation matrix.
//-----------------------------------------------------------------------------
D3DXMATRIX *SceneObject::GetRotationMatrix()
{
	return &m_rotationMatrix;
}

//-----------------------------------------------------------------------------
// Returns a pointer to the object's current world matrix.
//-----------------------------------------------------------------------------
D3DXMATRIX *SceneObject::GetWorldMatrix()
{
	return &m_worldMatrix;
}

//-----------------------------------------------------------------------------
// Returns a pointer to the object's current view matrix.
//-----------------------------------------------------------------------------
D3DXMATRIX *SceneObject::GetViewMatrix()
{
	return &m_viewMatrix;
}

//-----------------------------------------------------------------------------
// Sets the object's type.
//-----------------------------------------------------------------------------
void SceneObject::SetType( unsigned long type )
{
	m_type = type;
}

//-----------------------------------------------------------------------------
// Returns the object's type.
//-----------------------------------------------------------------------------
unsigned long SceneObject::GetType()
{
	return m_type;
}

//-----------------------------------------------------------------------------
// Sets the object's friction.
//-----------------------------------------------------------------------------
void SceneObject::SetFriction( float friction )
{
	m_friction = friction;
}

//-----------------------------------------------------------------------------
// Returns the collision stamp.
//-----------------------------------------------------------------------------
unsigned long SceneObject::GetCollisionStamp()
{
	return m_collisionStamp;
}

//-----------------------------------------------------------------------------
// Sets the object's visible flag.
//-----------------------------------------------------------------------------
void SceneObject::SetVisible( bool visible )
{
	m_visible = visible;
}

//-----------------------------------------------------------------------------
// Returns the object's visible flag.
//-----------------------------------------------------------------------------
bool SceneObject::GetVisible()
{
	return m_visible;
}

//-----------------------------------------------------------------------------
// Sets the object's enabled flag.
//-----------------------------------------------------------------------------
void SceneObject::SetEnabled( bool enabled )
{
	m_enabled = enabled;
}

//-----------------------------------------------------------------------------
// Returns the object's enabled flag.
//-----------------------------------------------------------------------------
bool SceneObject::GetEnabled()
{
	return m_enabled;
}

//-----------------------------------------------------------------------------
// Sets the object's ghost flag.
//-----------------------------------------------------------------------------
void SceneObject::SetGhost( bool ghost )
{
	m_ghost = ghost;
}

//-----------------------------------------------------------------------------
// Returns the object's ghost flag.
//-----------------------------------------------------------------------------
bool SceneObject::GetGhost()
{
	return m_ghost;
}

//-----------------------------------------------------------------------------
// Sets the object's ignore collisions flag.
//-----------------------------------------------------------------------------
void SceneObject::SetIgnoreCollisions( bool ignoreCollisions )
{
	m_ignoreCollisions = ignoreCollisions;
}

//-----------------------------------------------------------------------------
// Returns the object's ignore collisions flag.
//-----------------------------------------------------------------------------
bool SceneObject::GetIgnoreCollisions()
{
	return m_ignoreCollisions;
}

//-----------------------------------------------------------------------------
// Set's the the flag for touching the ground.
//-----------------------------------------------------------------------------
void SceneObject::SetTouchingGroundFlag( bool touchingGround )
{
	m_touchingGround = touchingGround;
}

//-----------------------------------------------------------------------------
// Returns true if the object is touching the ground.
//-----------------------------------------------------------------------------
bool SceneObject::IsTouchingGround()
{
	return m_touchingGround;
}

//-----------------------------------------------------------------------------
// Sets the mesh for this scene object.
//-----------------------------------------------------------------------------
void SceneObject::SetMesh( char *meshName, char *meshPath, bool sharedMesh )
{
	// Destroy the object's exisiting mesh.
	if( m_sharedMesh == true )
		g_engine->GetMeshManager()->Remove( &m_mesh );
	else
		SAFE_DELETE( m_mesh );

	// Indicate if the object is sharing this mesh.
	m_sharedMesh = sharedMesh;

	// Ensure a mesh was specified.
	if( meshName != NULL && meshPath != NULL )
	{
		// Load the object's mesh.
		if( m_sharedMesh == true )
			m_mesh = g_engine->GetMeshManager()->Add( meshName, meshPath );
		else
			m_mesh = new Mesh( meshName, meshPath );

		// Clone the mesh's bounding volume. The bounding volume will be used
		// to maintain an axis aligned bounding volume in world space.
		CloneBoundingVolume( m_mesh->GetBoundingBox(), m_mesh->GetBoundingSphere() );
	}
}

//-----------------------------------------------------------------------------
// Returns a pointer to the object's mesh.
//-----------------------------------------------------------------------------
Mesh *SceneObject::GetMesh()
{
	return m_mesh;
}