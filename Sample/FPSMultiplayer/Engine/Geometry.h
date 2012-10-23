//-----------------------------------------------------------------------------
// Various geometry structures and processing functions.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef GEOMETRY_H
#define GEOMETRY_H

//-----------------------------------------------------------------------------
// Vertex Structure
//-----------------------------------------------------------------------------
struct Vertex
{
	D3DXVECTOR3 translation; // Translation of the vertex (in world space).
	D3DXVECTOR3 normal; // Vertex's normal vector.
	float tu, tv; // Texture UV coordinates.

	//-------------------------------------------------------------------------
	// The vertex structure constructor.
	//-------------------------------------------------------------------------
	Vertex()
	{
		translation = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		normal = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		tu = 0.0f;
		tv = 0.0f;
	}

	//-------------------------------------------------------------------------
	// The vertex structure constructor.
	//-------------------------------------------------------------------------
	Vertex( D3DXVECTOR3 t, D3DXVECTOR3 n, float u, float v )
	{
		translation = t;
		normal = n;
		tu = u;
		tv = v;
	}
};
#define VERTEX_FVF ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 )
#define VERTEX_FVF_SIZE D3DXGetFVFVertexSize( VERTEX_FVF )

//-----------------------------------------------------------------------------
// Lit Vertex Structure
//-----------------------------------------------------------------------------
struct LVertex
{
	D3DXVECTOR3 translation; // Translation of the vertex (in world space).
	D3DCOLOR diffuse; // Colour of the vertex.
	float tu, tv; // Texture UV coordinates.

	//-------------------------------------------------------------------------
	// The lit vertex structure constructor.
	//-------------------------------------------------------------------------
	LVertex()
	{
		translation = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		diffuse = 0xFFFFFFFF;
		tu = 0.0f;
		tv = 0.0f;
	}

	//-------------------------------------------------------------------------
	// The lit vertex structure constructor.
	//-------------------------------------------------------------------------
	LVertex( D3DXVECTOR3 t, D3DCOLOR d, float u, float v )
	{
		translation = t;
		diffuse = d;
		tu = u;
		tv = v;
	}
};
#define L_VERTEX_FVF ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 )
#define L_VERTEX_FVF_SIZE D3DXGetFVFVertexSize( L_VERTEX_FVF )

//-----------------------------------------------------------------------------
// Transformed & Lit Vertex Structure
//-----------------------------------------------------------------------------
struct TLVertex
{
	D3DXVECTOR4 translation; // Translation of the vertex (in screen space).
	D3DCOLOR diffuse; // Colour of the vertex.
	float tu, tv; // Texture UV coordinates.

	//-------------------------------------------------------------------------
	// The transformed & lit vertex structure constructor.
	//-------------------------------------------------------------------------
	TLVertex()
	{
		translation = D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 1.0f );
		diffuse = 0xFFFFFFFF;
		tu = 0.0f;
		tv = 0.0f;
	}

	//-------------------------------------------------------------------------
	// The transformed & lit vertex structure constructor.
	//-------------------------------------------------------------------------
	TLVertex( D3DXVECTOR4 t, D3DCOLOR d, float u, float v )
	{
		translation = t;
		diffuse = d;
		tu = u;
		tv = v;
	}
};
#define TL_VERTEX_FVF ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 )
#define TL_VERTEX_FVF_SIZE D3DXGetFVFVertexSize( TL_VERTEX_FVF )

//-----------------------------------------------------------------------------
// Edge Structure
//-----------------------------------------------------------------------------
struct Edge
{
	Vertex *vertex0; // First vertex of the edge.
	Vertex *vertex1; // Second vertex of the edge.

	//-------------------------------------------------------------------------
	// The edge structure constructor.
	//-------------------------------------------------------------------------
	Edge( Vertex *v0, Vertex *v1 )
	{
		vertex0 = v0;
		vertex1 = v1;
	}
};

//-----------------------------------------------------------------------------
// Indexed Edge Structure
//-----------------------------------------------------------------------------
struct IndexedEdge
{
	unsigned short vertex0; // Index of the edge's first vertex.
	unsigned short vertex1; // Index of the edge's second vertex.
};

//-----------------------------------------------------------------------------
// Face Structure
//-----------------------------------------------------------------------------
struct Face
{
	Vertex *vertex0; // First vertex of the face.
	Vertex *vertex1; // Second vertex of the face.
	Vertex *vertex2; // Third vertex of the face.

	//-------------------------------------------------------------------------
	// The face structure constructor.
	//-------------------------------------------------------------------------
	Face( Vertex *v0, Vertex *v1, Vertex *v2 )
	{
		vertex0 = v0;
		vertex1 = v1;
		vertex2 = v2;
	}
};

//-----------------------------------------------------------------------------
// Indexed Face Structure
//-----------------------------------------------------------------------------
struct IndexedFace
{
	unsigned short vertex0; // Index of the face's first vertex.
	unsigned short vertex1; // Index of the face's second vertex.
	unsigned short vertex2; // Index of the face's third vertex.
};

//-----------------------------------------------------------------------------
// Returns true if the first given box is inside the second given box.
//-----------------------------------------------------------------------------
inline bool IsBoxInBox( D3DXVECTOR3 box1Min, D3DXVECTOR3 box1Max, D3DXVECTOR3 box2Min, D3DXVECTOR3 box2Max )
{
	if( box1Min.x > box2Max.x )
		return false;
	if( box1Min.y > box2Max.y )
		return false;
	if( box1Min.z > box2Max.z )
		return false;
	if( box1Max.x < box2Min.x )
		return false;
	if( box1Max.y < box2Min.y )
		return false;
	if( box1Max.z < box2Min.z )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Returns true if the given face is inside the given box.
//-----------------------------------------------------------------------------
inline bool IsFaceInBox( Vertex *vertex0, Vertex *vertex1, Vertex *vertex2, D3DXVECTOR3 boxMin, D3DXVECTOR3 boxMax )
{
	// Find the minimum and maximum points of the face along the x axis. Then
	// check if these two points are within the box's x axis extents.
	float minX = min( vertex0->translation.x, min( vertex1->translation.x, vertex2->translation.x ) );
	float maxX = max( vertex0->translation.x, max( vertex1->translation.x, vertex2->translation.x ) );
	if( maxX < boxMin.x )
		return false;
	if( minX > boxMax.x )
		return false;

	// Find the minimum and maximum points of the face along the y axis. Then
	// check if these two points are within the box's y axis extents.
	float minY = min( vertex0->translation.y, min( vertex1->translation.y, vertex2->translation.y ) );
	float maxY = max( vertex0->translation.y, max( vertex1->translation.y, vertex2->translation.y ) );
	if( maxY < boxMin.y )
		return false;
	if( minY > boxMax.y )
		return false;

	// Find the minimum and maximum points of the face along the z axis. Then
	// check if these two points are within the box's z axis extents.
	float minZ = min( vertex0->translation.z, min( vertex1->translation.z, vertex2->translation.z ) );
	float maxZ = max( vertex0->translation.z, max( vertex1->translation.z, vertex2->translation.z ) );
	if( maxZ < boxMin.z )
		return false;
	if( minZ > boxMax.z )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Returns true if the given box is completely enclosed by the given volume.
//-----------------------------------------------------------------------------
inline bool IsBoxEnclosedByVolume( LinkedList< D3DXPLANE > *planes, D3DXVECTOR3 min, D3DXVECTOR3 max )
{
	planes->Iterate( true );
	while( planes->Iterate() )
	{
		if( D3DXPlaneDotCoord( planes->GetCurrent(), &D3DXVECTOR3( min.x, min.y, min.z ) ) < 0.0f )
			return false;
		if( D3DXPlaneDotCoord( planes->GetCurrent(), &D3DXVECTOR3( max.x, min.y, min.z ) ) < 0.0f )
			return false;
		if( D3DXPlaneDotCoord( planes->GetCurrent(), &D3DXVECTOR3( min.x, max.y, min.z ) ) < 0.0f )
			return false;
		if( D3DXPlaneDotCoord( planes->GetCurrent(), &D3DXVECTOR3( max.x, max.y, min.z ) ) < 0.0f )
			return false;
		if( D3DXPlaneDotCoord( planes->GetCurrent(), &D3DXVECTOR3( min.x, min.y, max.z ) ) < 0.0f )
			return false;
		if( D3DXPlaneDotCoord( planes->GetCurrent(), &D3DXVECTOR3( max.x, min.y, max.z ) ) < 0.0f )
			return false;
		if( D3DXPlaneDotCoord( planes->GetCurrent(), &D3DXVECTOR3( min.x, max.y, max.z ) ) < 0.0f )
			return false;
		if( D3DXPlaneDotCoord( planes->GetCurrent(), &D3DXVECTOR3( max.x, max.y, max.z ) ) < 0.0f )
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Returns true if the given sphere is overlapping the given volume.
//-----------------------------------------------------------------------------
inline bool IsSphereOverlappingVolume( LinkedList< D3DXPLANE > *planes, D3DXVECTOR3 translation, float radius )
{
	planes->Iterate( true );
	while( planes->Iterate() )
		if( D3DXPlaneDotCoord( planes->GetCurrent(), &translation ) < -radius )
			return false;

	return true;
}

//-----------------------------------------------------------------------------
// Returns true if the two given spheres collide.
//-----------------------------------------------------------------------------
inline bool IsSphereCollidingWithSphere( float *collisionDistance, D3DXVECTOR3 translation1, D3DXVECTOR3 translation2, D3DXVECTOR3 velocitySum, float radiiSum )
{
	// Get the distance between the two spheres.
	float distanceBetween = D3DXVec3Length( &( translation1 - translation2 ) ) - radiiSum;

	// Get the length of the sum of the velocity vectors of the two spheres.
	float velocityLength = D3DXVec3Length( &velocitySum );

	// If the spheres are not touching each other and the velocity length is
	// less than the distance between them, then they cannot collide.
	if( distanceBetween > 0.0f && velocityLength < distanceBetween )
		return false;

	// Get the normalized sum of the velocity vectors.
	D3DXVECTOR3 normalizedVelocity;
	D3DXVec3Normalize( &normalizedVelocity, &velocitySum );

	// Get the direction vector from the second sphere to the first sphere.
	D3DXVECTOR3 direction = translation1 - translation2;

	// Get the angle between the normalized velocity and direction vectors.
	float angleBetween = D3DXVec3Dot( &normalizedVelocity, &direction );

	// Check if the spheres are moving away from one another.
	if( angleBetween <= 0.0f )
	{
		// Check if they are touching (or inside) each other. If not then they
		// cannot collide since they are moving away from one another.
		if( distanceBetween < 0.0f )
		{
			// If the velocity length is greater than the distance between the
			// spheres then they are moving away from each other fast enough
			// that they will not be touching when they complete their move.
			if( velocityLength > -distanceBetween )
				return false;
		}
		else
			return false;
	}

	// Get the length of the direction vector.
	float directionLength = D3DXVec3Length( &direction );

	// The vector between the two spheres and the velocity vector produce two
	// sides of a triangle. Now use Pythagorean Theorem to find the length of
	// the third side of the triangle (i.e. the hypotenuse).
	float hypotenuse = ( directionLength * directionLength ) - ( angleBetween * angleBetween );

	// Ensure that the spheres come closer than the sum of their radii.
	float radiiSumSquared = radiiSum * radiiSum;
	if( hypotenuse >= radiiSumSquared )
		return false;

	// Get the distance along the velocity vector that the spheres collide.
	// Then use this distance to calculate the distance to the collision.
	float distance = radiiSumSquared - hypotenuse;
	*collisionDistance = angleBetween - (float)sqrt( distance );

	// Ensure that the sphere will not travel more than the velocity allows.
	if( velocityLength < *collisionDistance )
		return false;

	return true;
}

#endif