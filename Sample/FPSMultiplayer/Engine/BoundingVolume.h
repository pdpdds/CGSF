//-----------------------------------------------------------------------------
// Used for storing a bounding volume in terms of a bounding box and a bounding
// sphere. Additionally, the bounding sphere can be viewed as a 3D ellipsoid.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef BOUNDING_VOLUME_H
#define BOUNDING_VOLUME_H

//-----------------------------------------------------------------------------
// Bounding Box Structure
//-----------------------------------------------------------------------------
struct BoundingBox
{
	D3DXVECTOR3 min; // Minimum extent of the bounding box.
	D3DXVECTOR3 max; // Maximum extent of the bounding box.
	float halfSize; // Distance from the centre of the volume to the furthest point on any axis.
};

//-----------------------------------------------------------------------------
// Bounding Sphere Structure
//-----------------------------------------------------------------------------
struct BoundingSphere
{
	D3DXVECTOR3 centre; // Centre point of the bounding sphere.
	float radius; // Radius of the bounding sphere.
};

//-----------------------------------------------------------------------------
// Bounding Volume Class
//-----------------------------------------------------------------------------
class BoundingVolume
{
public:
	BoundingVolume();
	virtual ~BoundingVolume();

	void BoundingVolumeFromMesh( ID3DXMesh *mesh, D3DXVECTOR3 ellipsoidRadius = D3DXVECTOR3( 1.0f, 1.0f, 1.0f ) );
	void BoundingVolumeFromVertices( D3DXVECTOR3 *vertices, unsigned long totalVertices, unsigned long vertexStride, D3DXVECTOR3 ellipsoidRadius = D3DXVECTOR3( 1.0f, 1.0f, 1.0f ) );
	void CloneBoundingVolume( BoundingBox *box, BoundingSphere *sphere, D3DXVECTOR3 ellipsoidRadius = D3DXVECTOR3( 1.0f, 1.0f, 1.0f ) );
	void RepositionBoundingVolume( D3DXMATRIX *location );

	void SetBoundingBox( D3DXVECTOR3 min, D3DXVECTOR3 max );
	BoundingBox *GetBoundingBox();

	void SetBoundingSphere( D3DXVECTOR3 centre, float radius, D3DXVECTOR3 ellipsoidRadius = D3DXVECTOR3( 1.0f, 1.0f, 1.0f ) );
	BoundingSphere *GetBoundingSphere();

	void SetEllipsoidRadius( D3DXVECTOR3 ellipsoidRadius );
	D3DXVECTOR3 GetEllipsoidRadius();

private:
	BoundingBox *m_box; // Box representation of the bounding volume.
	BoundingSphere *m_sphere; // Sphere representation of the bounding volume.

	D3DXVECTOR3 m_originalMin; // Original minimum extents of the bounding box.
	D3DXVECTOR3 m_originalMax; // Original maximum extents of the bounding box.
	D3DXVECTOR3 m_originalCentre; // Original centre point of the bounding sphere.

	D3DXVECTOR3 m_ellipsoidRadius; // Ellipsoid radius (i.e. the radius along each axis).
};

#endif