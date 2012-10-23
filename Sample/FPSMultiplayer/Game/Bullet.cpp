//-----------------------------------------------------------------------------
// Bullet.h implementation.
// Refer to the Bullet.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "Main.h"

//-----------------------------------------------------------------------------
// The bullet class constructor.
//-----------------------------------------------------------------------------
Bullet::Bullet( SceneObject *owner, D3DXVECTOR3 translation, D3DXVECTOR3 direction, float velocity, float range, float damage )
{
	// Store the owner of the bullet.
	m_owner = owner;

	// Create a ray intersection result for the bullet.
	m_hitResult = new RayIntersectionResult;

	// Indicate that the bullet has not moved yet.
	m_totalDistance = 0.0f;

	// The bullet has not expired yet.
	m_expired = false;

	// Set all the properties of the bullet.
	m_translation = translation;
	m_direction = direction;
	m_velocity = velocity;
	m_range = range;
	m_damage = damage;
}

//-----------------------------------------------------------------------------
// The bullet class destructor.
//-----------------------------------------------------------------------------
Bullet::~Bullet()
{
	SAFE_DELETE( m_hitResult );
}

//-----------------------------------------------------------------------------
// Updates the bullet.
//-----------------------------------------------------------------------------
void Bullet::Update( float elapsed )
{
	// Clear the hit result.
	m_hitResult->material = NULL;
	m_hitResult->distance = 0.0f;
	m_hitResult->hitObject = NULL;

	// Cast a ray through the scene to see if the bullet might hit something.
	if( g_engine->GetSceneManager()->RayIntersectScene( m_hitResult, m_translation, m_direction, true, m_owner, true ) == true )
	{
		// Ensure the hit distance is within the velocity of the bullet.
		if( m_hitResult->distance <= m_velocity * elapsed )
		{
			// Ensure the bullet will not go beyond its range.
			m_totalDistance += m_hitResult->distance;
			if( m_totalDistance > m_range )
			{
				// Clear the hit result.
				m_hitResult->material = NULL;
				m_hitResult->distance = 0.0f;
				m_hitResult->hitObject = NULL;

				// Indicate that the bullet has expired.
				m_expired = true;

				// Return now so that the collision isn't processed.
				return;
			}

			// The bullet has legally hit something, so expire it.
			m_expired = true;

			// Only the host is allowed to process bullet collision results.
			//if( g_engine->GetNetwork()->IsHost() == false )
				//return;

			// Ensure the bullet hit an object, and not just part of the scene.
			if( m_hitResult->hitObject == NULL )
				return;

			// Check if the bullet hit another player.
			if( m_hitResult->hitObject->GetType() == TYPE_PLAYER_OBJECT )
				( (PlayerObject*)m_hitResult->hitObject )->Hurt( m_damage, (PlayerObject*)m_owner );

			return;
		}
		else
		{
			// Clear the hit result.
			m_hitResult->material = NULL;
			m_hitResult->distance = 0.0f;
			m_hitResult->hitObject = NULL;
		}
	}

	// Expire the bullet if it will move beyond its range.
	m_totalDistance += m_velocity * elapsed;
	if( m_totalDistance > m_range )
	{
		m_expired = true;

		return;
	}

	// Move the bullet.
	m_translation += m_direction * ( m_velocity * elapsed );
}

//-----------------------------------------------------------------------------
// Returns true if the bullet is expired.
//-----------------------------------------------------------------------------
bool Bullet::IsExpired()
{
	return m_expired;
}

//-----------------------------------------------------------------------------
// The bullet manager class constructor.
//-----------------------------------------------------------------------------
BulletManager::BulletManager()
{
	// Create the linked list of bullets.
	m_bullets = new LinkedList< Bullet >;
}

//-----------------------------------------------------------------------------
// The bullet manager class destructor.
//-----------------------------------------------------------------------------
BulletManager::~BulletManager()
{
	SAFE_DELETE( m_bullets );
}

//-----------------------------------------------------------------------------
// Allows the bullet manager to update its bullets.
//-----------------------------------------------------------------------------
void BulletManager::Update( float elapsed )
{
	// Go through the list of bullets.
	Bullet *remove = NULL;
	Bullet *bullet = m_bullets->GetFirst();
	while( bullet != NULL )
	{
		// Check if the bullet has expired. If not, allow it to update.
		if( bullet->IsExpired() == true )
			remove = bullet;
		else
			bullet->Update( elapsed );

		// Go to the next bullet.
		bullet = m_bullets->GetNext( bullet );

		// If the bullet did expire, then remove it now.
		if( remove != NULL )
			m_bullets->Remove( &remove );
	}
}

//-----------------------------------------------------------------------------
// Adds a new bullet to the bullet manager.
//-----------------------------------------------------------------------------
void BulletManager::AddBullet( SceneObject *owner, D3DXVECTOR3 translation, D3DXVECTOR3 direction, float velocity, float range, float damage )
{
	m_bullets->Add( new Bullet( owner, translation, direction, velocity, range, damage ) );
}