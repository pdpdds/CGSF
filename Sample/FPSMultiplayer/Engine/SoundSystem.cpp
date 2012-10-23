//-----------------------------------------------------------------------------
// SoundSystem.h implementation.
// Refer to the SoundSystem.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "Engine.h"

//-----------------------------------------------------------------------------
// The sound system class constructor.
//-----------------------------------------------------------------------------
SoundSystem::SoundSystem( float scale )
{
	// Create the DirectMusic loader.
	CoCreateInstance( CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, IID_IDirectMusicLoader8, (void**)&m_loader );

	// Create and initialise the DirectMusic performance.
	CoCreateInstance( CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC, IID_IDirectMusicPerformance8, (void**)&m_performance );
	m_performance->InitAudio( NULL, NULL, NULL, DMUS_APATH_SHARED_STEREOPLUSREVERB, 8, DMUS_AUDIOF_ALL, NULL );

	// Get the 3D listener by creating a 3D audio path then retrieving the
	// listener from the audio path. The audio path can then be released.
	IDirectMusicAudioPath8 *audioPath3D;
	m_performance->CreateStandardAudioPath( DMUS_APATH_DYNAMIC_3D, 1, true, &audioPath3D );
	audioPath3D->GetObjectInPath( 0, DMUS_PATH_PRIMARY_BUFFER, 0, GUID_All_Objects, 0, IID_IDirectSound3DListener, (void**)&m_listener );
	SAFE_RELEASE( audioPath3D );

	// Set the scale (and the distance factor).
	m_scale = scale;
	m_listener->SetDistanceFactor( m_scale, DS3D_IMMEDIATE );
}

//-----------------------------------------------------------------------------
// The sound system class destructor.
//-----------------------------------------------------------------------------
SoundSystem::~SoundSystem()
{
	// Close down and release the DirectMusic performance.
	m_performance->CloseDown();
	SAFE_RELEASE( m_performance );

	// Release the DirectMusic loader.
	SAFE_RELEASE( m_loader );
}

//-----------------------------------------------------------------------------
// Updates the sound system's 3D listener.
//-----------------------------------------------------------------------------
void SoundSystem::UpdateListener( D3DXVECTOR3 forward, D3DXVECTOR3 position, D3DXVECTOR3 velocity )
{
	// Set the listener's orientation.
	m_listener->SetOrientation( forward.x, forward.y, forward.z, 0.0f, 1.0f, 0.0f, DS3D_DEFERRED );

	// Scale and set the listener's position.
	position *= m_scale;
	m_listener->SetPosition( position.x, position.y, position.z, DS3D_DEFERRED );

	// Scale and set the listener's velocity.
	velocity *= m_scale;
	m_listener->SetVelocity( velocity.x, velocity.y, velocity.z, DS3D_DEFERRED );

	// Commit the settings.
	m_listener->CommitDeferredSettings();
}

//-----------------------------------------------------------------------------
// Allows the sound system to remove unused sound objects.
//-----------------------------------------------------------------------------
void SoundSystem::GarbageCollection()
{
	m_loader->CollectGarbage();
}

//-----------------------------------------------------------------------------
// Sets the master volume for all sounds played through this sound system.
//-----------------------------------------------------------------------------
void SoundSystem::SetVolume( long volume )
{
	m_performance->SetGlobalParam( GUID_PerfMasterVolume, &volume, sizeof( long ) );
}

//-----------------------------------------------------------------------------
// Returns a pointer to the sound system's loader object.
//-----------------------------------------------------------------------------
IDirectMusicLoader8 *SoundSystem::GetLoader()
{
	return m_loader;
}

//-----------------------------------------------------------------------------
// Returns the sound system's performance object.
//-----------------------------------------------------------------------------
IDirectMusicPerformance8 *SoundSystem::GetPerformance()
{
	return m_performance;
}

//-----------------------------------------------------------------------------
// The sound class constructor.
//-----------------------------------------------------------------------------
Sound::Sound( char *filename )
{
	// Convert the filename into a wide character string.
	WCHAR *wideFilename = new WCHAR[strlen( filename ) + 1];
	MultiByteToWideChar( CP_ACP, 0, filename, -1, wideFilename, strlen( filename ) + 1 );
	wideFilename[strlen( filename )] = 0;

	// Load the sound file, then destroy the file name.
	g_engine->GetSoundSystem()->GetLoader()->LoadObjectFromFile( CLSID_DirectMusicSegment, IID_IDirectMusicSegment8, wideFilename, (void**)&m_segment );
	SAFE_DELETE( wideFilename );

	// Download the segment's data to the perfromance.
	m_segment->Download( g_engine->GetSoundSystem()->GetPerformance() );
}

//-----------------------------------------------------------------------------
// The sound class destructor.
//-----------------------------------------------------------------------------
Sound::~Sound()
{
	g_engine->GetSoundSystem()->GetPerformance()->Stop(m_segment, NULL, 0, 0);
	// Unload the segment's data from the perfromance.
	m_segment->Unload( g_engine->GetSoundSystem()->GetPerformance() );

	// Remove the segment from the DirectMusic loader, then release it.
	g_engine->GetSoundSystem()->GetLoader()->ReleaseObjectByUnknown( m_segment );
	SAFE_RELEASE( m_segment );
}

//-----------------------------------------------------------------------------
// Plays the sound on the standard audiopath with or without looping.
// Warning: Sound segments can only maintain a single looping state. Therefore
// changing the looping flag will effect all instances of this sound.
//-----------------------------------------------------------------------------
void Sound::Play( bool loop, unsigned long flags )
{
	if( loop == true )
		m_segment->SetRepeats( DMUS_SEG_REPEAT_INFINITE );
	else
		m_segment->SetRepeats( 0 );

	g_engine->GetSoundSystem()->GetPerformance()->PlaySegment( m_segment, flags, 0, NULL );
}

//-----------------------------------------------------------------------------
// Returns the sound's segment.
//-----------------------------------------------------------------------------
IDirectMusicSegment8 *Sound::GetSegment()
{
	return m_segment;
}

//-----------------------------------------------------------------------------
// The audio path 3D class constructor.
//-----------------------------------------------------------------------------
AudioPath3D::AudioPath3D()
{
	// Create an audiopath so the 3D parameters can be set individually.
	g_engine->GetSoundSystem()->GetPerformance()->CreateStandardAudioPath( DMUS_APATH_DYNAMIC_3D, 1, true, &m_audioPath );

	// Get the audiopath's sound buffer.
	m_audioPath->GetObjectInPath( DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, 0, GUID_NULL, 0, IID_IDirectSound3DBuffer, (void**)&m_soundBuffer );
}

//-----------------------------------------------------------------------------
// The audio path 3D class destructor.
//-----------------------------------------------------------------------------
AudioPath3D::~AudioPath3D()
{
	SAFE_RELEASE( m_soundBuffer );
	SAFE_RELEASE( m_audioPath );
}

//-----------------------------------------------------------------------------
// Sets the position of the sound in 3D space.
//-----------------------------------------------------------------------------
void AudioPath3D::SetPosition( D3DXVECTOR3 position )
{
	position *= g_engine->GetScale();
	m_soundBuffer->SetPosition( position.x, position.y, position.z, DS3D_IMMEDIATE );
}

//-----------------------------------------------------------------------------
// Sets the velocity of the sound in 3D space.
//-----------------------------------------------------------------------------
void AudioPath3D::SetVelocity( D3DXVECTOR3 velocity )
{
	velocity *= g_engine->GetScale();
	m_soundBuffer->SetVelocity( velocity.x, velocity.y, velocity.z, DS3D_IMMEDIATE );
}

//-----------------------------------------------------------------------------
// Sets the mode that the audio path plays sounds in.
//-----------------------------------------------------------------------------
void AudioPath3D::SetMode( unsigned long mode )
{
	m_soundBuffer->SetMode( mode, DS3D_IMMEDIATE );
}

//-----------------------------------------------------------------------------
// Plays the given sound on the 3D audio path with or without looping.
// Warning: Sound segments can only maintain a single looping state. Therefore
// changing the looping flag will effect all instances of this sound.
//-----------------------------------------------------------------------------
void AudioPath3D::Play( IDirectMusicSegment8 *segment, bool loop, unsigned long flags )
{
	if( loop == true )
		segment->SetRepeats( DMUS_SEG_REPEAT_INFINITE );
	else
		segment->SetRepeats( 0 );

	g_engine->GetSoundSystem()->GetPerformance()->PlaySegmentEx( segment, NULL, NULL, flags, 0, NULL, NULL, m_audioPath );
}