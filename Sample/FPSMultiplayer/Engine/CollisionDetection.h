//-----------------------------------------------------------------------------
// Provides basic collision detection in a 3D environment that includes both
// static geometry and dynamic objects.
//
// Note: This uses an adaptation of the algorithm from the Improved Collision
//       Detection and Response article written by Kasper Fauerby.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef COLLISION_H
#define COLLISION_H

//-----------------------------------------------------------------------------
// Collision Data Structure
//-----------------------------------------------------------------------------
struct CollisionData
{
	float scale; // Scale used by thge calling scene manager.
	float elapsed; // Elapsed time for the current frame.
	unsigned long frameStamp; // Current frame stamp according to the scene manager.

	SceneObject *object; // Pointer to the object to perform collision detection with.

	D3DXVECTOR3 translation; // Translation in ellipsoid space.
	D3DXVECTOR3 velocity; // Velocity in ellipsoid space.
	D3DXVECTOR3 normalizedVelocity; // Normalized velocity in ellipsoid space.

	D3DXVECTOR3 gravity; // Gravity vector, which will be converted to ellipsoid space.

	bool collisionFound; // Indicates if a collision has been found.
	float distance; // Distance to the point of collision.
	D3DXVECTOR3 intersection; // Actual intersection point where the collision occured.
};

//-----------------------------------------------------------------------------
// Returns the lowest root of a quadratic equation.
//-----------------------------------------------------------------------------
inline float GetLowestRoot( float a, float b, float c, float max )
{
	// Calculate the determinant, then get the square root of it if it's valid.
	float determinant = b * b - a * c;
	if( determinant < 0.0f )
		return 0.0f;
	determinant = (float)sqrt( determinant );

	// Calculate the first root and ensure it is within the bounds.
	float root1 = ( b + determinant ) / a;
	if( root1 <= 0.0f || root1 > max )
		root1 = max + 1.0f;

	// Calculate the second root and ensure it is within the bounds.
	float root2 = ( b - determinant ) / a;
	if( root2 <= 0.0f || root2 > max )
		root2 = max + 1.0f;

	// Get the lowest of the two roots.
	float root = min( root1, root2 );

	// Ensure the root is valid.
	if( root == max + 1.0f )
		return 0.0f;

	return root;
}

//-----------------------------------------------------------------------------
// Checks a single face for intersection.
//-----------------------------------------------------------------------------
inline void CheckFace( CollisionData *data, D3DXVECTOR3 vertex0, D3DXVECTOR3 vertex1, D3DXVECTOR3 vertex2 )
{
	// Create a plane from the face's vertices.
	D3DXPLANE plane;
	D3DXPlaneFromPoints( &plane, &vertex0, &vertex1, &vertex2 );

	// Get the angle between the plane's normal and the velocity vector.
	float angle = D3DXPlaneDotNormal( &plane, &data->normalizedVelocity );

	// Ensure the plane is facing the velocity vector (i.e. ignore back faces).
	if( angle > 0.0f )
		return;

	// Get the plane's normal vector.
	D3DXVECTOR3 planeNormal;
	D3DXVec3Cross( &planeNormal, &( vertex0 - vertex1 ), &( vertex1 - vertex2 ) );
	D3DXVec3Normalize( &planeNormal, &planeNormal );

	// Calculate the signed distance from sphere's translation to plane.
	float signedPlaneDistance = D3DXVec3Dot( &data->translation, &planeNormal ) + plane.d;

	// Get interval of plane intersection
	float time0, time1;
	bool embedded = false;

	// Cache this as we're going to use it a few times below
	float normalDotVelocity = D3DXVec3Dot( &planeNormal, &data->velocity );

	// Check if the sphere is travelling parallel to the plane.
	if( normalDotVelocity == 0.0f )
	{
		// If the sphere is not embedded in the plane, then it cannot collide.
		if( fabs( signedPlaneDistance ) >= 1.0f )
			return;
		else
		{
			// The sphere is embedded in plane, therefore it will collide
			// for the entire time frame.
			embedded = true;
			time0 = 0.0f;
			time1 = 1.0f;
		}
	}
	else
	{
		// Calculate the time frame of intersection.
		time0 = ( -1.0f - signedPlaneDistance ) / normalDotVelocity;
		time1 = ( 1.0f - signedPlaneDistance ) / normalDotVelocity;

		// Ensure time0 is less than time1.
		if( time0 > time1 )
		{
			float swap = time1;
			time1 = time0;
			time0 = swap;
		}

		// If the intersection time frame is out of range, then it cannot collide.
		if( time0 > 1.0f || time1 < 0.0f )
			return;

		// Normalize the time frame.
		if( time0 < 0.0f ) time0 = 0.0f;
		if( time1 < 0.0f ) time1 = 0.0f;
		if( time0 > 1.0f ) time0 = 1.0f;
		if( time1 > 1.0f ) time1 = 1.0f;
	}

	// Variables used for tracking if an intersection occured, where it happened, and when.
	bool intersectFound = false;
	D3DXVECTOR3 intersection;
	float intersectTime = 1.0f;

	// Check if the sphere is embedded in the plane.
	if( embedded == false )
	{
		// Get the plane intersection point at time0.
		D3DXVECTOR3 planeIntersectionPoint = ( data->translation - planeNormal ) + data->velocity * time0;

		// Get the vectors of two of the face's edges.
		D3DXVECTOR3 edge0 = vertex1 - vertex0;
		D3DXVECTOR3 edge1 = vertex2 - vertex0;

		// Get the angles of the edges and combine them.
		float angle0 = D3DXVec3Dot( &edge0, &edge0 );
		float angle1 = D3DXVec3Dot( &edge0, &edge1 );
		float angle2 = D3DXVec3Dot( &edge1, &edge1 );
		float combined = ( angle0 * angle2 ) - ( angle1 * angle1 );

		// Get the split angles between the two edges.
		D3DXVECTOR3 split = planeIntersectionPoint - vertex0;
		float splitAngle0 = D3DXVec3Dot( &split, &edge0 );
		float splitAngle1 = D3DXVec3Dot( &split, &edge1 );

		float x = ( splitAngle0 * angle2 ) - ( splitAngle1 * angle1 );
		float y = ( splitAngle1 * angle0 ) - ( splitAngle0 * angle1 );
		float z = x + y - combined;

		// Take the bitwise AND of z and the complement of the inclusive OR of x and y,
		// then bitwise AND the result with 0x80000000 and return it. A bitwise result
		// of zero equals false, while any other value equals true.
		if( ( ( (unsigned int&)z & ~( (unsigned int&)x | (unsigned int&)y ) ) & 0x80000000 ) != 0 )
		{
			intersectFound = true;
			intersection = planeIntersectionPoint;
			intersectTime = time0;
		}
	}

	// Check if a collision has been found yet.
	if( intersectFound == false )
	{
		// Get the squared length of the velocity vector.
		float squaredVelocityLength = D3DXVec3LengthSq( &data->velocity );

		// A quadratic equation has to be solved for each vertex and edge in the face.
		// The following variables are used to build each quadratic equation.
		float a, b, c;

		// Used for storing the result of each quadratic equation.
		float newTime;

		// First check againts the vertices.
		a = squaredVelocityLength;

		// Check vertex 0.
		b = 2.0f * D3DXVec3Dot( &data->velocity, &( data->translation - vertex0 ) );
		c = D3DXVec3LengthSq( &( vertex0 - data->translation ) ) - 1.0f;
		if( newTime = GetLowestRoot( a, b, c, intersectTime ) > 0.0f )
		{
			intersectFound = true;
			intersection = vertex0;
			intersectTime = newTime;
		}

		// Check vertex 1.
		b = 2.0f * D3DXVec3Dot( &data->velocity, &( data->translation - vertex1 ) );
		c = D3DXVec3LengthSq( &( vertex1 - data->translation ) ) - 1.0f;
		if( newTime = GetLowestRoot( a, b, c, intersectTime ) > 0.0f )
		{
			intersectFound = true;
			intersection = vertex1;
			intersectTime = newTime;
		}

		// Check vertex 2.
		b = 2.0f * D3DXVec3Dot( &data->velocity, &( data->translation - vertex2 ) );
		c = D3DXVec3LengthSq( &( vertex2 - data->translation ) ) - 1.0f;
		if( newTime = GetLowestRoot( a, b, c, intersectTime ) > 0.0f )
		{
			intersectFound = true;
			intersection = vertex2;
			intersectTime = newTime;
		}

		// Check the edge from vertex0 to vertex1.
		D3DXVECTOR3 edge = vertex1 - vertex0;
		D3DXVECTOR3 vectorSphereVertex = vertex0 - data->translation;
		float squaredEdgeLength = D3DXVec3LengthSq( &edge );
		float angleEdgeVelocity = D3DXVec3Dot( &edge, &data->velocity );
		float angleEdgeSphereVertex = D3DXVec3Dot( &edge, &vectorSphereVertex );

		// Get the parameters for the quadratic equation.
		a = squaredEdgeLength * -squaredVelocityLength + angleEdgeVelocity * angleEdgeVelocity;
		b = squaredEdgeLength * ( 2.0f * D3DXVec3Dot( &data->velocity, &vectorSphereVertex ) ) - 2.0f * angleEdgeVelocity * angleEdgeSphereVertex;
		c = squaredEdgeLength * ( 1.0f - D3DXVec3LengthSq( &vectorSphereVertex ) ) + angleEdgeSphereVertex * angleEdgeSphereVertex;

		// Check if the sphere intersects the edge.
		if( newTime = GetLowestRoot( a, b, c, intersectTime ) > 0.0f )
		{
			// Ensure the intersection occured within the edges bounds.
			float point = ( angleEdgeVelocity * newTime - angleEdgeSphereVertex ) / squaredEdgeLength;
			if( point >= 0.0f && point <= 1.0f )
			{
				intersectFound = true;
				intersection = vertex0 + edge * point;
				intersectTime = newTime;
			}
		}

		// Check the edge from vertex1 to vertex2.
		edge = vertex2 - vertex1;
		vectorSphereVertex = vertex1 - data->translation;
		squaredEdgeLength = D3DXVec3LengthSq( &edge );
		angleEdgeVelocity = D3DXVec3Dot( &edge, &data->velocity );
		angleEdgeSphereVertex = D3DXVec3Dot( &edge, &vectorSphereVertex );

		// Get the parameters for the quadratic equation.
		a = squaredEdgeLength * -squaredVelocityLength + angleEdgeVelocity * angleEdgeVelocity;
		b = squaredEdgeLength * ( 2.0f * D3DXVec3Dot( &data->velocity, &vectorSphereVertex ) ) - 2.0f * angleEdgeVelocity * angleEdgeSphereVertex;
		c = squaredEdgeLength * ( 1.0f - D3DXVec3LengthSq( &vectorSphereVertex ) ) + angleEdgeSphereVertex * angleEdgeSphereVertex;

		// Check if the sphere intersects the edge.
		if( newTime = GetLowestRoot( a, b, c, intersectTime ) > 0.0f )
		{
			// Ensure the intersection occured within the edges bounds.
			float point = ( angleEdgeVelocity * newTime - angleEdgeSphereVertex ) / squaredEdgeLength;
			if( point >= 0.0f && point <= 1.0f )
			{
				intersectFound = true;
				intersection = vertex1 + edge * point;
				intersectTime = newTime;
			}
		}

		// Check the edge from vertex2 to vertex0.
		edge = vertex0 - vertex2;
		vectorSphereVertex = vertex2 - data->translation;
		squaredEdgeLength = D3DXVec3LengthSq( &edge );
		angleEdgeVelocity = D3DXVec3Dot( &edge, &data->velocity );
		angleEdgeSphereVertex = D3DXVec3Dot( &edge, &vectorSphereVertex );

		// Get the parameters for the quadratic equation.
		a = squaredEdgeLength * -squaredVelocityLength + angleEdgeVelocity * angleEdgeVelocity;
		b = squaredEdgeLength * ( 2.0f * D3DXVec3Dot( &data->velocity, &vectorSphereVertex ) ) - 2.0f * angleEdgeVelocity * angleEdgeSphereVertex;
		c = squaredEdgeLength * ( 1.0f - D3DXVec3LengthSq( &vectorSphereVertex ) ) + angleEdgeSphereVertex * angleEdgeSphereVertex;

		// Check if the sphere intersects the edge.
		if( newTime = GetLowestRoot( a, b, c, intersectTime ) > 0.0f )
		{
			// Ensure the intersection occured within the edges bounds.
			float point = ( angleEdgeVelocity * newTime - angleEdgeSphereVertex ) / squaredEdgeLength;
			if( point >= 0.0f && point <= 1.0f )
			{
				intersectFound = true;
				intersection = vertex2 + edge * point;
				intersectTime = newTime;
			}
		}
	}

	// Check if an intersection occured.
	if( intersectFound == true )
	{
		// Get the distance to the collision (i.e. time along the velocity vector).
		float collisionDistance = intersectTime * D3DXVec3Length( &data->velocity );

		// Store the collision details, if necessary.
		if( data->collisionFound == false || collisionDistance < data->distance )
		{
			data->distance = collisionDistance;
			data->intersection = intersection;
			data->collisionFound = true;
		}
	}
}

//-----------------------------------------------------------------------------
// Perfrom collision detection between the give object and the scene.
//-----------------------------------------------------------------------------
inline void CollideWithScene( CollisionData *data, Vertex *vertices, SceneFace *faces, unsigned long totalFaces, LinkedList< SceneObject > *objects, unsigned long recursion = 5 )
{
	// Calculate the epsilon distance (taking scale into account).
	// The epsilon distance is a very short distance that is considered negligable.
	float epsilon = 0.5f * data->scale;

	// Indicate that a collision has not been found.
	data->collisionFound = false;

	// Get the normalized velocity vector.
	D3DXVec3Normalize( &data->normalizedVelocity, &data->velocity );

	// Go through all of the faces.
	D3DXVECTOR3 vertex0, vertex1, vertex2;
	for( unsigned long f = 0; f < totalFaces; f++ )
	{
		// Skip this face if its material is set to ignore rays.
		if( faces[f].renderCache->GetMaterial()->GetIgnoreRay() == true )
			continue;

		// Get a copy of this face's vertices in ellipsoid space.
		vertex0.x = vertices[faces[f].vertex0].translation.x / data->object->GetEllipsoidRadius().x;
		vertex0.y = vertices[faces[f].vertex0].translation.y / data->object->GetEllipsoidRadius().y;
		vertex0.z = vertices[faces[f].vertex0].translation.z / data->object->GetEllipsoidRadius().z;
		vertex1.x = vertices[faces[f].vertex1].translation.x / data->object->GetEllipsoidRadius().x;
		vertex1.y = vertices[faces[f].vertex1].translation.y / data->object->GetEllipsoidRadius().y;
		vertex1.z = vertices[faces[f].vertex1].translation.z / data->object->GetEllipsoidRadius().z;
		vertex2.x = vertices[faces[f].vertex2].translation.x / data->object->GetEllipsoidRadius().x;
		vertex2.y = vertices[faces[f].vertex2].translation.y / data->object->GetEllipsoidRadius().y;
		vertex2.z = vertices[faces[f].vertex2].translation.z / data->object->GetEllipsoidRadius().z;

		// Check for collision with this face.
		CheckFace( data, vertex0, vertex1, vertex2 );
	}

	// Create a list of hit ghost objects and a list of the distances to them.
	LinkedList< SceneObject > *ghostHits = new LinkedList< SceneObject >;
	LinkedList< float > *ghostDistances = new LinkedList< float >;

	// Variables used for the following object collision check.
	D3DXVECTOR3 translation, velocity, vectorColliderObject, vectorObjectCollider, vectorObjectRadius;
	float distToCollision, colliderRadius, objectRadius;

	// Go through the list of objects.
	SceneObject *hitObject = NULL;
	SceneObject *nextObject = objects->GetFirst();
	while( nextObject != NULL )
	{
		// Skip this object if it is the collider. It can't check against itself.
		if( nextObject != data->object )
		{
			// Get the translation and velocity of this object in ellipsoid space.
			translation.x = nextObject->GetTranslation().x / data->object->GetEllipsoidRadius().x;
			translation.y = nextObject->GetTranslation().y / data->object->GetEllipsoidRadius().y;
			translation.z = nextObject->GetTranslation().z / data->object->GetEllipsoidRadius().z;
			velocity.x = nextObject->GetVelocity().x / data->object->GetEllipsoidRadius().x;
			velocity.y = nextObject->GetVelocity().y / data->object->GetEllipsoidRadius().y;
			velocity.z = nextObject->GetVelocity().z / data->object->GetEllipsoidRadius().z;
			velocity *= data->elapsed;

			// Get the normalized vectors from the collider to this object and vice versa.
			D3DXVec3Normalize( &vectorColliderObject, &( translation - data->translation ) );
			D3DXVec3Normalize( &vectorObjectCollider, &( data->translation - translation ) );

			// Calculate the radius of each ellipsoid in the direction to the other.
			colliderRadius = D3DXVec3Length( &vectorColliderObject );
			vectorObjectRadius.x = vectorObjectCollider.x * nextObject->GetEllipsoidRadius().x / data->object->GetEllipsoidRadius().x;
			vectorObjectRadius.y = vectorObjectCollider.y * nextObject->GetEllipsoidRadius().y / data->object->GetEllipsoidRadius().y;
			vectorObjectRadius.z = vectorObjectCollider.z * nextObject->GetEllipsoidRadius().z / data->object->GetEllipsoidRadius().z;
			objectRadius = D3DXVec3Length( &vectorObjectRadius );

			// Check for collision between the two spheres.
			if( IsSphereCollidingWithSphere( &distToCollision, data->translation, translation, velocity - data->velocity, colliderRadius + objectRadius ) == true )
			{
				// Check if the hit object is a ghost.
				if( nextObject->GetGhost() == true )
				{
					// If both object's are allowed to register collisions, then store a pointer to the hit object and the distance to hit it.
					if( nextObject->GetIgnoreCollisions() == false && data->object->GetIgnoreCollisions() == false )
					{
						ghostHits->Add( nextObject );
						ghostDistances->Add( &distToCollision );
					}
				}
				else
				{
					// Store the collision details, if necessary.
					if( data->collisionFound == false || distToCollision < data->distance )
					{
						data->distance = distToCollision;
						data->intersection = data->normalizedVelocity * distToCollision;
						data->collisionFound = true;

						// Store a pointer to the hit object.
						hitObject = nextObject;
					}
				}
			}
		}

		// Go to the next object.
		nextObject = objects->GetNext( nextObject );
	}

	// Iterate through the list of hit ghost objects and their collision distances.
	ghostHits->Iterate( true );
	ghostDistances->Iterate( true );
	while( ghostHits->Iterate() != NULL && ghostDistances->Iterate() != NULL )
	{
		// If the distance to hit the ghost object is less than the distance to the closets real collision, then the ghost object has been hit.
		if( *ghostDistances->GetCurrent() < data->distance )
		{
			// Register the collision between both objects.
			ghostHits->GetCurrent()->CollisionOccurred( data->object, data->frameStamp );
			data->object->CollisionOccurred( ghostHits->GetCurrent(), data->frameStamp );
		}
	}

	// Destroy the ghost hits and distances lists.
	ghostHits->ClearPointers();
	SAFE_DELETE( ghostHits );
	ghostDistances->ClearPointers();
	SAFE_DELETE( ghostDistances );

	// If no collision occured, then just move the full velocity vector.
	if( data->collisionFound == false )
	{
		data->translation = data->translation + data->velocity;
		return;
	}

	// Calculate the destination (i.e. the point the object was trying to get to).
	D3DXVECTOR3 destination = data->translation + data->velocity;

	// The new translation will be the point where the object actually ends up.
	D3DXVECTOR3 newTranslation = data->translation;

	// Ignore the movement if the object is already very close to its destination.
	if( data->distance >= epsilon )
	{
		// Calculate the new velocity required to move the distance.
		D3DXVECTOR3 newVelocity = data->normalizedVelocity * ( data->distance - epsilon );

		// Find the new translation.
		newTranslation = data->translation + newVelocity;

		// Adjust the polygon intersection point to taking into account that fact that
		// the object does not move right up to the actual intersection point.
		D3DXVec3Normalize( &newVelocity, &newVelocity );
		data->intersection = data->intersection - newVelocity * epsilon;
	}

	// Check if the collision occured with an object.
	if( hitObject != NULL )
	{
		// Set the new translation of the object.
		data->translation = newTranslation;

		// Calculate and apply a push velocity so objects can push one another.
		D3DXVECTOR3 push = ( hitObject->GetVelocity() + data->object->GetVelocity() ) / 10.0f;
		hitObject->SetVelocity( push );
		data->object->SetVelocity( push );

		// Register the collision between both objects, if thay are allowed.
		if( hitObject->GetIgnoreCollisions() == false && data->object->GetIgnoreCollisions() == false )
		{
			hitObject->CollisionOccurred( data->object, data->frameStamp );
			data->object->CollisionOccurred( hitObject, data->frameStamp );
		}

		return;
	}

	// Create a plane that wil act as the sliding plane.
	D3DXVECTOR3 slidePlaneOrigin = data->intersection;
	D3DXVECTOR3 slidePlaneNormal;
	D3DXVec3Normalize( &slidePlaneNormal, &( newTranslation - data->intersection ) );
	D3DXPLANE slidingPlane;
	D3DXPlaneFromPointNormal( &slidingPlane, &slidePlaneOrigin, &slidePlaneNormal );

	// Calculate the new destination accouting for sliding.
	D3DXVECTOR3 newDestination = destination - slidePlaneNormal * ( D3DXVec3Dot( &destination, &slidePlaneNormal ) + slidingPlane.d );
	newDestination += slidePlaneNormal * epsilon;

	// Calculate the new velocity which is the vector of the slide.
	D3DXVECTOR3 newVelocity = newDestination - data->intersection;

	// Check if the new velocity is too short.
	if( D3DXVec3Length( &newVelocity ) <= epsilon )
	{
		// Since the velocity is too short, there is no need to continue
		// performing collision detection. So just set the new translation
		// and velocity, then return.
		data->translation = newTranslation;
		data->velocity = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

		return;
	}

	// Set the new translation and velocity.
	data->translation = newTranslation;
	data->velocity = newVelocity;

	// Perform another collision detection recurison if allowed.
	recursion--;
	if( recursion > 0 )
		CollideWithScene( data, vertices, faces, totalFaces, objects, recursion );
}

//-----------------------------------------------------------------------------
// Entry point for collision detection and response.
//-----------------------------------------------------------------------------
inline void PerformCollisionDetection( CollisionData *data, Vertex *vertices, SceneFace *faces, unsigned long totalFaces, LinkedList< SceneObject > *dynamicObjects )
{
	// Calculate the object's translation in ellipsoid space.
	data->translation.x = data->object->GetTranslation().x / data->object->GetEllipsoidRadius().x;
	data->translation.y = data->object->GetTranslation().y / data->object->GetEllipsoidRadius().y;
	data->translation.z = data->object->GetTranslation().z / data->object->GetEllipsoidRadius().z;

	// Calculate the object's velocity in ellipsoid space.
	data->velocity = data->object->GetVelocity() * data->elapsed;
	data->velocity.x /= data->object->GetEllipsoidRadius().x;
	data->velocity.y /= data->object->GetEllipsoidRadius().y;
	data->velocity.z /= data->object->GetEllipsoidRadius().z;

	// Begin the recursive collision detection.
	CollideWithScene( data, vertices, faces, totalFaces, dynamicObjects );

	// Now set the velocity to the gravity vector (in ellipsoid space).
	data->velocity.x = data->gravity.x / data->object->GetEllipsoidRadius().x;
	data->velocity.y = data->gravity.y / data->object->GetEllipsoidRadius().y;
	data->velocity.z = data->gravity.z / data->object->GetEllipsoidRadius().z;

	// Perform another recursive collision detection to apply gravity.
	CollideWithScene( data, vertices, faces, totalFaces, dynamicObjects );

	// Convert the object's new translation back out of ellipsoid space.
	data->translation.x = data->translation.x * data->object->GetEllipsoidRadius().x;
	data->translation.y = data->translation.y * data->object->GetEllipsoidRadius().y;
	data->translation.z = data->translation.z * data->object->GetEllipsoidRadius().z;

	// Go through all the faces in the scene, checking for intersection.
	float hitDistance = -1.0f;
	for( unsigned long f = 0; f < totalFaces; f++ )
	{
		// Skip this face if its material is set to ignore rays.
		if( faces[f].renderCache->GetMaterial()->GetIgnoreRay() == true )
			continue;

		// Preform a ray intersection test to see if this face is under the object.
		float distance;
		if( D3DXIntersectTri( (D3DXVECTOR3*)&vertices[faces[f].vertex0], (D3DXVECTOR3*)&vertices[faces[f].vertex1], (D3DXVECTOR3*)&vertices[faces[f].vertex2], &data->translation, &D3DXVECTOR3( 0.0f, -1.0f, 0.0f ), NULL, NULL, &distance ) == TRUE )
			if( distance < hitDistance || hitDistance == -1.0f )
				hitDistance = distance;
	}

	// If the distance to the ray intersection is less than the radius
	// of the object along the y axis, then the object is embedded in
	// the ground. So just push the object up out of the ground.
	if( hitDistance < data->object->GetEllipsoidRadius().y )
		data->translation.y += data->object->GetEllipsoidRadius().y - hitDistance;

	// Check if the object is touching the ground.
	if( hitDistance < data->object->GetEllipsoidRadius().y + 0.1f / data->scale )
		data->object->SetTouchingGroundFlag( true );
	else
		data->object->SetTouchingGroundFlag( false );

	// Update the object's translation after collision detection.
	data->object->SetTranslation( data->translation );
}

#endif