//-----------------------------------------------------------------------------
// AnimatedObject.h implementation.
// Refer to the AnimatedObject.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "Engine.h"

//-----------------------------------------------------------------------------
// The animated object class constructor.
//-----------------------------------------------------------------------------
AnimatedObject::AnimatedObject( char *meshName, char *meshPath, unsigned long type ) : SceneObject( type, meshName, meshPath, false )
{
	// Create a clone of the mesh's animation controller.
	if( GetMesh() != NULL )
		GetMesh()->CloneAnimationController( &m_animationController );
	else
		m_animationController = NULL;

	// Set the track speed on both tracks to full speed.
	if( m_animationController != NULL )
	{
		m_animationController->SetTrackSpeed( 0, 1.0f );
		m_animationController->SetTrackSpeed( 1, 1.0f );
	}

	// Clear the variables used for animation.
	m_currentTrack = 0;
	m_currentTime = 0.0f;
}

//-----------------------------------------------------------------------------
// The animated object class destructor.
//-----------------------------------------------------------------------------
AnimatedObject::~AnimatedObject()
{
	SAFE_RELEASE( m_animationController );
}

//-----------------------------------------------------------------------------
// Updates the object.
//-----------------------------------------------------------------------------
void AnimatedObject::Update( float elapsed, bool addVelocity )
{
	// Allow the base scene object to update.
	SceneObject::Update( elapsed, addVelocity );

	// Check if the object has an animation controller.
	if( m_animationController )
	{
		// Advance the object's animation controller.
		m_animationController->AdvanceTime( elapsed, this );

		// Keep track of the current time for animation purposes.
		m_currentTime += elapsed;
	}

	// Update the mesh.
	if( GetMesh() != NULL )
		GetMesh()->Update();
}

//-----------------------------------------------------------------------------
// Plays the given animation with the given transition time.
//-----------------------------------------------------------------------------
void AnimatedObject::PlayAnimation( unsigned int animation, float transitionTime, bool loop )
{
	// Ensure object has a valid animation controller.
	if( m_animationController == NULL )
		return;

	// Ensure the transition time is always greater than zero.
	if( transitionTime <= 0.0f )
		transitionTime = 0.000001f;

	// Find which track to play the new animation on.
	unsigned int newTrack = ( m_currentTrack == 0 ? 1 : 0 );

	// Get a pointer to the animation set to play.
	ID3DXAnimationSet *as;
	m_animationController->GetAnimationSet( animation, &as );

	// Set the animation set to the new track.
	m_animationController->SetTrackAnimationSet( newTrack, as );

	// Clear all the events that are currently set on the tracks.
	m_animationController->UnkeyAllTrackEvents( m_currentTrack );
	m_animationController->UnkeyAllTrackEvents( newTrack );

	// Check if this animation should be looped or only played once.
	if( loop == true )
	{
		// Transition the new track in over the specified transition time period.
		m_animationController->KeyTrackEnable( m_currentTrack, false, m_currentTime + transitionTime );
		m_animationController->KeyTrackWeight( m_currentTrack, 0.0f, m_currentTime, transitionTime, D3DXTRANSITION_LINEAR );
		m_animationController->SetTrackEnable( newTrack, true );
		m_animationController->KeyTrackWeight( newTrack, 1.0f, m_currentTime, transitionTime, D3DXTRANSITION_LINEAR );
	}
	else
	{
		// Stop the current track, and start the new track without transitioning.
		m_animationController->SetTrackEnable( m_currentTrack, false );
		m_animationController->SetTrackWeight( m_currentTrack, 0.0f );
		m_animationController->SetTrackEnable( newTrack, true );
		m_animationController->SetTrackWeight( newTrack, 1.0f );
		m_animationController->SetTrackPosition( newTrack, 0.0f );
		m_animationController->KeyTrackEnable( newTrack, false, m_currentTime + as->GetPeriod() );
	}

	// Release the pointer to the animation set.
	as->Release();

	// The new track is now the current track.
	m_currentTrack = newTrack;
}

//-----------------------------------------------------------------------------
// Returns a pointer to the object's animation controller.
//-----------------------------------------------------------------------------
ID3DXAnimationController *AnimatedObject::GetAnimationController()
{
	return m_animationController;
}

//-----------------------------------------------------------------------------
// Animation callback handler.
//-----------------------------------------------------------------------------
HRESULT CALLBACK AnimatedObject::HandleCallback( THIS_ UINT Track, LPVOID pCallbackData )
{
	return S_OK;
}