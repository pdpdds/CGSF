//-----------------------------------------------------------------------------
// Functionality for a bullet to exist in the scene. The bullet manager tracks
// and processes all of the bullets.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef BULLET_H
#define BULLET_H

//-----------------------------------------------------------------------------
// Bullet Class
//-----------------------------------------------------------------------------
class Bullet
{
public:
	Bullet( SceneObject *owner, D3DXVECTOR3 translation, D3DXVECTOR3 direction, float velocity, float range, float damage );
	virtual ~Bullet();

	void Update( float elapsed );

	bool IsExpired();

private:
	SceneObject *m_owner; // Owner of the bullet. Used to prevent players from shooting themselves.
	RayIntersectionResult *m_hitResult; // Result of the bullet's ray intersection.
	float m_totalDistance; // Total distance covered by the bullet.
	bool m_expired; // Indicates if the bullet has expired yet.

	D3DXVECTOR3 m_translation; // Current translation of the bullet.
	D3DXVECTOR3 m_direction; // Direction the bullet is travelling.
	float m_velocity; // Velocity of the bullet.
	float m_range; // How far the bullet can travel.
	float m_damage; // How much damage the bullet does.
};

//-----------------------------------------------------------------------------
// Bullet Manager Class
//-----------------------------------------------------------------------------
class BulletManager
{
public:
	BulletManager();
	virtual ~BulletManager();

	void Update( float elapsed );

	void AddBullet( SceneObject *owner, D3DXVECTOR3 translation, D3DXVECTOR3 direction, float velocity, float range, float damage );

private:
	LinkedList< Bullet > *m_bullets; // Linked list of bullets.
};

#endif