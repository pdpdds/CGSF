//-----------------------------------------------------------------------------
// A derived scene object that supports mesh animation.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef ANIMATED_OBJECT_H
#define ANIMATED_OBJECT_H

//-----------------------------------------------------------------------------
// Animated Object Type Define
//-----------------------------------------------------------------------------
#define TYPE_ANIMATED_OBJECT 1

//-----------------------------------------------------------------------------
// Animated Object Class
//-----------------------------------------------------------------------------
class AnimatedObject : public SceneObject, public ID3DXAnimationCallbackHandler
{
public:
	AnimatedObject( char *meshName, char *meshPath = "./", unsigned long type = TYPE_ANIMATED_OBJECT );
	virtual ~AnimatedObject();

	virtual void Update( float elapsed, bool addVelocity = true );

	void PlayAnimation( unsigned int animation, float transitionTime, bool loop = true );
	ID3DXAnimationController *GetAnimationController();

private:
	virtual HRESULT CALLBACK HandleCallback( THIS_ UINT Track, LPVOID pCallbackData );

private:
	ID3DXAnimationController *m_animationController; // Controller for managing mesh animation playback.
	unsigned int m_currentTrack; // The track of the currently playing animation.
	float m_currentTime; // Timer used for animation.
};

#endif