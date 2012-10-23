//-----------------------------------------------------------------------------
// BoundingVolume.h implementation.
// Refer to the BoundingVolume.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "Engine.h"

//-----------------------------------------------------------------------------
// The bounding volume class constructor.
//-----------------------------------------------------------------------------
BoundingVolume::BoundingVolume()
{
	m_box = new BoundingBox;
	m_sphere = new BoundingSphere;

	m_ellipsoidRadius = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
}

//-----------------------------------------------------------------------------
// The bounding volume class destructor.
//-----------------------------------------------------------------------------
BoundingVolume::~BoundingVolume()
{
	SAFE_DELETE( m_box );
	SAFE_DELETE( m_sphere );
}

//-----------------------------------------------------------------------------
// Builds a bounding volume to enclose the given mesh.
//-----------------------------------------------------------------------------
void BoundingVolume::BoundingVolumeFromMesh( ID3DXMesh *mesh, D3DXVECTOR3 ellipsoidRadius )
{
	D3DXVECTOR3 *vertices;
	if( SUCCEEDED( mesh->LockVertexBuffer( D3DLOCK_READONLY, (void**)&vertices ) ) )
	{
		D3DXComputeBoundingBox( vertices, mesh->GetNumVertices(), D3DXGetFVFVertexSize( mesh->GetFVF() ), &m_box->min, &m_box->max );
		D3DXComputeBoundingSphere( vertices, mesh->GetNumVertices(), D3DXGetFVFVertexSize( mesh->GetFVF() ), &m_sphere->centre, &m_sphere->radius );
		mesh->UnlockVertexBuffer();
	}

	m_sphere->centre.x = m_box->min.x + ( ( m_box->max.x - m_box->min.x ) / 2.0f );
	m_sphere->centre.y = m_box->min.y + ( ( m_box->max.y - m_box->min.y ) / 2.0f );
	m_sphere->centre.z = m_box->min.z + ( ( m_box->max.z - m_box->min.z ) / 2.0f );

	m_box->halfSize = (float)max( fabs( m_box->max.x ), max( fabs( m_box->max.y ), fabs( m_box->max.z ) ) );
	m_box->halfSize = (float)max( m_box->halfSize, max( fabs( m_box->min.x ), max( fabs( m_box->min.y ), fabs( m_box->min.z ) ) ) );

	m_originalMin = m_box->min;
	m_originalMax = m_box->max;
	m_originalCentre = m_sphere->centre;

	SetEllipsoidRadius( ellipsoidRadius );
}

//-----------------------------------------------------------------------------
// Builds a bounding volume to enclose the given vertices.
//-----------------------------------------------------------------------------
void BoundingVolume::BoundingVolumeFromVertices( D3DXVECTOR3 *vertices, unsigned long totalVertices, unsigned long vertexStride, D3DXVECTOR3 ellipsoidRadius )
{
	D3DXComputeBoundingBox( vertices, totalVertices, vertexStride, &m_box->min, &m_box->max );
	D3DXComputeBoundingSphere( vertices, totalVertices, vertexStride, &m_sphere->centre, &m_sphere->radius );

	m_sphere->centre.x = m_box->min.x + ( ( m_box->max.x - m_box->min.x ) / 2.0f );
	m_sphere->centre.y = m_box->min.y + ( ( m_box->max.y - m_box->min.y ) / 2.0f );
	m_sphere->centre.z = m_box->min.z + ( ( m_box->max.z - m_box->min.z ) / 2.0f );

	m_box->halfSize = (float)max( fabs( m_box->max.x ), max( fabs( m_box->max.y ), fabs( m_box->max.z ) ) );
	m_box->halfSize = (float)max( m_box->halfSize, max( fabs( m_box->min.x ), max( fabs( m_box->min.y ), fabs( m_box->min.z ) ) ) );

	m_originalMin = m_box->min;
	m_originalMax = m_box->max;
	m_originalCentre = m_sphere->centre;

	SetEllipsoidRadius( ellipsoidRadius );
}

//-----------------------------------------------------------------------------
// Builds a bounding volume from the given bounding volume details.
//-----------------------------------------------------------------------------
void BoundingVolume::CloneBoundingVolume( BoundingBox *box, BoundingSphere *sphere, D3DXVECTOR3 ellipsoidRadius )
{
	m_box->min = box->min;
	m_box->max = box->max;
	m_sphere->centre = sphere->centre;
	m_sphere->radius = sphere->radius;

	m_box->halfSize = (float)max( fabs( m_box->max.x ), max( fabs( m_box->max.y ), fabs( m_box->max.z ) ) );
	m_box->halfSize = (float)max( m_box->halfSize, max( fabs( m_box->min.x ), max( fabs( m_box->min.y ), fabs( m_box->min.z ) ) ) );

	m_originalMin = m_box->min;
	m_originalMax = m_box->max;
	m_originalCentre = m_sphere->centre;

	SetEllipsoidRadius( ellipsoidRadius );
}

//-----------------------------------------------------------------------------
// Repositions the bounding volume by the given matrix.
//-----------------------------------------------------------------------------
void BoundingVolume::RepositionBoundingVolume( D3DXMATRIX *location )
{
	D3DXVec3TransformCoord( &m_box->min, &m_originalMin, location );
	D3DXVec3TransformCoord( &m_box->max, &m_originalMax, location );
	D3DXVec3TransformCoord( &m_sphere->centre, &m_originalCentre, location );
}

//-----------------------------------------------------------------------------
// Sets the bounding box's properties.
//-----------------------------------------------------------------------------
void BoundingVolume::SetBoundingBox( D3DXVECTOR3 min, D3DXVECTOR3 max )
{
	m_originalMin = m_box->min = min;
	m_originalMax = m_box->max = max;

	m_box->halfSize = (float)max( fabs( m_box->max.x ), max( fabs( m_box->max.y ), fabs( m_box->max.z ) ) );
	m_box->halfSize = (float)max( m_box->halfSize, max( fabs( m_box->min.x ), max( fabs( m_box->min.y ), fabs( m_box->min.z ) ) ) );
}

//-----------------------------------------------------------------------------
// Returns the bounding box.
//-----------------------------------------------------------------------------
BoundingBox *BoundingVolume::GetBoundingBox()
{
	return m_box;
}

//-----------------------------------------------------------------------------
// Sets the bounding sphere's properties.
//-----------------------------------------------------------------------------
void BoundingVolume::SetBoundingSphere( D3DXVECTOR3 centre, float radius, D3DXVECTOR3 ellipsoidRadius )
{
	m_originalCentre = m_sphere->centre = centre;
	m_sphere->radius = radius;

	SetEllipsoidRadius( ellipsoidRadius );
}

//-----------------------------------------------------------------------------
// Returns the bounding sphere.
//-----------------------------------------------------------------------------
BoundingSphere *BoundingVolume::GetBoundingSphere()
{
	return m_sphere;
}

//-----------------------------------------------------------------------------
// Sets the ellipsoid radius to a percentage of the sphere radius.
//-----------------------------------------------------------------------------
void BoundingVolume::SetEllipsoidRadius( D3DXVECTOR3 ellipsoidRadius )
{
	m_ellipsoidRadius = D3DXVECTOR3( m_sphere->radius * ellipsoidRadius.x, m_sphere->radius * ellipsoidRadius.y, m_sphere->radius * ellipsoidRadius.z );
}

//-----------------------------------------------------------------------------
// Returns the ellipsoid radius.
//-----------------------------------------------------------------------------
D3DXVECTOR3 BoundingVolume::GetEllipsoidRadius()
{
	return m_ellipsoidRadius;
}