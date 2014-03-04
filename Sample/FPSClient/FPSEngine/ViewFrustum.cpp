//-----------------------------------------------------------------------------
// ViewFrustum.h implementation.
// Refer to the ViewFrustum.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "Engine.h"

//-----------------------------------------------------------------------------
// Creates a view frustum from the given view matrix.
//-----------------------------------------------------------------------------
void ViewFrustum::Update( D3DXMATRIX *view )
{
	// Calculate the field of view.
	D3DXMATRIX fov;
	D3DXMatrixMultiply( &fov, view, &m_projection );

	// Calculate the right plane.
	m_planes[0].a = fov._14 - fov._11;
	m_planes[0].b = fov._24 - fov._21;
	m_planes[0].c = fov._34 - fov._31;
	m_planes[0].d = fov._44 - fov._41;

	// Calculate the left plane.
	m_planes[1].a = fov._14 + fov._11;
	m_planes[1].b = fov._24 + fov._21;
	m_planes[1].c = fov._34 + fov._31;
	m_planes[1].d = fov._44 + fov._41;

	// Calculate the top plane.
	m_planes[2].a = fov._14 - fov._12;
	m_planes[2].b = fov._24 - fov._22;
	m_planes[2].c = fov._34 - fov._32;
	m_planes[2].d = fov._44 - fov._42;

	// Calculate the bottom plane.
	m_planes[3].a = fov._14 + fov._12;
	m_planes[3].b = fov._24 + fov._22;
	m_planes[3].c = fov._34 + fov._32;
	m_planes[3].d = fov._44 + fov._42;

	// Calculate the far plane.
	m_planes[4].a = fov._14 - fov._13;
	m_planes[4].b = fov._24 - fov._23;
	m_planes[4].c = fov._34 - fov._33;
	m_planes[4].d = fov._44 - fov._43;

	// Normalize the planes.
	D3DXPlaneNormalize( &m_planes[0], &m_planes[0] );
	D3DXPlaneNormalize( &m_planes[1], &m_planes[1] );
	D3DXPlaneNormalize( &m_planes[2], &m_planes[2] );
	D3DXPlaneNormalize( &m_planes[3], &m_planes[3] );
	D3DXPlaneNormalize( &m_planes[4], &m_planes[4] );
}

//-----------------------------------------------------------------------------
// Set's the view frustum's internal projection matrix.
//-----------------------------------------------------------------------------
void ViewFrustum::SetProjectionMatrix( D3DXMATRIX projection )
{
	m_projection = projection;
}

//-----------------------------------------------------------------------------
// Returns true if the given box is inside the view frustum.
//-----------------------------------------------------------------------------
bool ViewFrustum::IsBoxInside( D3DXVECTOR3 min, D3DXVECTOR3 max )
{
	for( char p = 0; p < 5; p++ )
	{
		if( D3DXPlaneDotCoord( &m_planes[p], &D3DXVECTOR3( min.x, min.y, min.z ) ) >= 0.0f )
			continue;
		if( D3DXPlaneDotCoord( &m_planes[p], &D3DXVECTOR3( max.x, min.y, min.z ) ) >= 0.0f )
			continue;
		if( D3DXPlaneDotCoord( &m_planes[p], &D3DXVECTOR3( min.x, max.y, min.z ) ) >= 0.0f )
			continue;
		if( D3DXPlaneDotCoord( &m_planes[p], &D3DXVECTOR3( max.x, max.y, min.z ) ) >= 0.0f )
			continue;
		if( D3DXPlaneDotCoord( &m_planes[p], &D3DXVECTOR3( min.x, min.y, max.z ) ) >= 0.0f )
			continue;
		if( D3DXPlaneDotCoord( &m_planes[p], &D3DXVECTOR3( max.x, min.y, max.z ) ) >= 0.0f )
			continue;
		if( D3DXPlaneDotCoord( &m_planes[p], &D3DXVECTOR3( min.x, max.y, max.z ) ) >= 0.0f )
			continue;
		if( D3DXPlaneDotCoord( &m_planes[p], &D3DXVECTOR3( max.x, max.y, max.z ) ) >= 0.0f )
			continue;

		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Returns true if the given box is inside the view frustum.
//-----------------------------------------------------------------------------
bool ViewFrustum::IsBoxInside( D3DXVECTOR3 translation, D3DXVECTOR3 min, D3DXVECTOR3 max )
{
	return IsBoxInside( min + translation, max + translation );
}

//-----------------------------------------------------------------------------
// Returns true if the given sphere is inside the view frustum.
//-----------------------------------------------------------------------------
bool ViewFrustum::IsSphereInside( D3DXVECTOR3 translation, float radius )
{
	for( char p = 0; p < 5; p++ )
		if( D3DXPlaneDotCoord( &m_planes[p], &translation ) < -radius )
			return false;

	return true;
}