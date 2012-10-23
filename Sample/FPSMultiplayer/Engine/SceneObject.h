//-----------------------------------------------------------------------------
// The minimum requirements for an object to exist in a 3D scene. Application
// specific objects must derive from this class.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

//-----------------------------------------------------------------------------
// Scene Object Type Define
//-----------------------------------------------------------------------------
#define TYPE_SCENE_OBJECT 0

//-----------------------------------------------------------------------------
// Scene Object Class
//-----------------------------------------------------------------------------
class SceneObject : public BoundingVolume
{
public:
	SceneObject( unsigned long type = TYPE_SCENE_OBJECT, char *meshName = NULL, char *meshPath = "./", bool sharedMesh = true );
	virtual ~SceneObject();

	virtual void Update( float elapsed, bool addVelocity = true );
	virtual void Render( D3DXMATRIX *world = NULL );

	virtual void CollisionOccurred( SceneObject *object, unsigned long collisionStamp );

	void Drive( float force, bool lockYAxis = true );
	void Strafe( float force, bool lockYAxis = true );
	void Stop();

	void SetTranslation( float x, float y, float z );
	void SetTranslation( D3DXVECTOR3 translation );
	void AddTranslation( float x, float y, float z );
	void AddTranslation( D3DXVECTOR3 translation );
	D3DXVECTOR3 GetTranslation();

	void SetRotation( float x, float y, float z );
	void SetRotation( D3DXVECTOR3 rotation );
	void AddRotation( float x, float y, float z );
	void AddRotation( D3DXVECTOR3 rotation );
	D3DXVECTOR3 GetRotation();

	void SetVelocity( float x, float y, float z );
	void SetVelocity( D3DXVECTOR3 velocity );
	void AddVelocity( float x, float y, float z );
	void AddVelocity( D3DXVECTOR3 velocity );
	D3DXVECTOR3 GetVelocity();

	void SetSpin( float x, float y, float z );
	void SetSpin( D3DXVECTOR3 spin );
	void AddSpin( float x, float y, float z );
	void AddSpin( D3DXVECTOR3 spin );
	D3DXVECTOR3 GetSpin();

	D3DXVECTOR3 GetForwardVector();
	D3DXVECTOR3 GetRightVector();

	D3DXMATRIX *GetTranslationMatrix();
	D3DXMATRIX *GetRotationMatrix();
	D3DXMATRIX *GetWorldMatrix();
	D3DXMATRIX *GetViewMatrix();

	void SetType( unsigned long type );
	unsigned long GetType();

	void SetFriction( float friction );

	unsigned long GetCollisionStamp();

	void SetVisible( bool visible );
	bool GetVisible();

	void SetEnabled( bool enabled );
	bool GetEnabled();

	void SetGhost( bool ghost );
	bool GetGhost();

	void SetIgnoreCollisions( bool ignoreCollisions );
	bool GetIgnoreCollisions();

	void SetTouchingGroundFlag( bool touchingGround );
	bool IsTouchingGround();

	void SetMesh( char *meshName = NULL, char *meshPath = "./", bool sharedMesh = true );
	Mesh *GetMesh();

protected:
	D3DXVECTOR3 m_forward; // Object's forward vector.
	D3DXVECTOR3 m_right; // Object's right vector.

	D3DXMATRIX m_worldMatrix; // World matrix.
	D3DXMATRIX m_viewMatrix; // View matrix.

private:
	D3DXVECTOR3 m_translation; // Object's translation in 3D space.
	D3DXVECTOR3 m_rotation; // Object's rotation in radians.

	D3DXVECTOR3 m_velocity; // Object's velocity in units/second.
	D3DXVECTOR3 m_spin; // Object's spin in radians/second.

	D3DXMATRIX m_translationMatrix; // Translation matrix.
	D3DXMATRIX m_rotationMatrix; // Rotation matrix.

	unsigned long m_type; // Identifies the scene object's parent class.
	float m_friction; // Friction applied to the object's velocity and spin.
	unsigned long m_collisionStamp; // Indicates the last frame when a collision occurred.
	bool m_visible; // Indicates it the object is visible. Invisible objects are not rendered.
	bool m_enabled; // Indicates if the object is enabled. Disabled objects are not updated.
	bool m_ghost; // Indicates if the object is a ghost. Ghost objects cannot physically collide with anything.
	bool m_ignoreCollisions; // Indicates if the object is to ignore collisions. Physical collisions can still occur, they're just not registered.
	bool m_touchingGround; // Indicates if the object is touching the ground.
	bool m_sharedMesh; // Indicates if the object is sharing the mesh or has exclusive access.
	Mesh *m_mesh; // Pointer to the object's mesh.
};

#endif