//-----------------------------------------------------------------------------
// SceneManager.h implementation.
// Refer to the SceneManager.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "Engine.h"

//-----------------------------------------------------------------------------
// The scene manager class constructor.
//-----------------------------------------------------------------------------
SceneManager::SceneManager( float scale, char *spawnerPath )
{
	m_name = NULL;
	m_scale = scale;
	m_gravity = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_loaded = false;
	m_mesh = NULL;
	m_maxFaces = 0;
	m_maxHalfSize = 0.0f;
	m_frameStamp = 0;

	m_dynamicObjects = new LinkedList< SceneObject >;
	m_occludingObjects = NULL;
	m_visibleOccluders = NULL;
	m_playerSpawnPoints = NULL;
	m_objectSpawners = NULL;
	m_spawnerPath = spawnerPath;

	m_firstLeaf = NULL;

	m_sceneVertexBuffer = NULL;
	m_vertices = NULL;
	m_totalVertices = 0;

	m_renderCaches = NULL;

	m_totalFaces = 0;
	m_faces = NULL;
}

//-----------------------------------------------------------------------------
// The scene manager class destructor.
//-----------------------------------------------------------------------------
SceneManager::~SceneManager()
{
	DestroyScene();

	// Destroy the dynamic objects list as it was created in the constructor.
	SAFE_DELETE( m_dynamicObjects );
}

//-----------------------------------------------------------------------------
// Loads a new scene from the given scene file.
//-----------------------------------------------------------------------------
void SceneManager::LoadScene( char *name, char *path )
{
	// Create the lists of objects used in the scene. The dynamic object list
	// is persistent across scene changes so it doesn't need to be created.
	m_occludingObjects = new LinkedList< SceneOccluder >;
	m_visibleOccluders = new LinkedList< SceneOccluder >;
	m_playerSpawnPoints = new LinkedList< SceneObject >;
	m_objectSpawners = new LinkedList< SpawnerObject >;

	// Load the script for the scene.
	Script *script = new Script( name, path );

	// Store the name of the scene.
	m_name = new char[strlen( script->GetStringData( "name" ) ) + 1];
	memcpy( m_name, script->GetStringData( "name" ), ( strlen( script->GetStringData( "name" ) ) + 1 ) * sizeof( char ) );

	// Store the scene's gravity vector.
	m_gravity = *script->GetVectorData( "gravity" ) / m_scale;

	// Create the sun light source.
	D3DLIGHT9 sun;
	sun.Type = D3DLIGHT_DIRECTIONAL;
	sun.Diffuse.r = 1.0f;
	sun.Diffuse.g = 1.0f;
	sun.Diffuse.b = 1.0f;
	sun.Diffuse.a = 1.0f;
	sun.Specular = sun.Diffuse;
	sun.Ambient.r = script->GetColourData( "ambient_light" )->r;
	sun.Ambient.g = script->GetColourData( "ambient_light" )->g;
	sun.Ambient.b = script->GetColourData( "ambient_light" )->b;
	sun.Ambient.a = script->GetColourData( "ambient_light" )->a;
	sun.Direction = D3DXVECTOR3( script->GetVectorData( "sun_direction" )->x, script->GetVectorData( "sun_direction" )->y, script->GetVectorData( "sun_direction" )->z );
	sun.Range = 0.0f;

	// Switch lighting on, enable the sun light, and specular highlights.
	g_engine->GetDevice()->SetRenderState( D3DRS_LIGHTING, true );
	g_engine->GetDevice()->SetLight( 0, &sun );
	g_engine->GetDevice()->LightEnable( 0, true );
	g_engine->GetDevice()->SetRenderState( D3DRS_SPECULARENABLE, true );

	// Set up the fog.
	float density = *script->GetFloatData( "fog_density" ) * m_scale;
	g_engine->GetDevice()->SetRenderState( D3DRS_FOGENABLE, true );
	g_engine->GetDevice()->SetRenderState( D3DRS_FOGCOLOR, D3DCOLOR_COLORVALUE( script->GetColourData( "fog_colour" )->r, script->GetColourData( "fog_colour" )->g, script->GetColourData( "fog_colour" )->b, script->GetColourData( "fog_colour" )->a ) );
	g_engine->GetDevice()->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_EXP2 );
	g_engine->GetDevice()->SetRenderState( D3DRS_FOGDENSITY, *(unsigned long*)&density );

	// Store the constraints used for creating the scene.
	m_maxFaces = *script->GetNumberData( "max_faces" );
	m_maxHalfSize = *script->GetFloatData( "max_half_size" );

	// Load the scene's mesh.
	m_mesh = g_engine->GetMeshManager()->Add( script->GetStringData( "mesh" ), script->GetStringData( "mesh_path" ) );

	// Destory the scene's script as it is no longer needed.
	SAFE_DELETE( script );

	// Set a new projection matrix so the view frustum will fit the scene.
	D3DDISPLAYMODE *display;
	display = g_engine->GetDisplayMode();
	D3DXMATRIX projection;
	D3DXMatrixPerspectiveFovLH( &projection, D3DX_PI / 4, (float)display->Width / (float)display->Height, 0.1f / m_scale, m_mesh->GetBoundingSphere()->radius * 2.0f );
	g_engine->GetDevice()->SetTransform( D3DTS_PROJECTION, &projection );

	// Set the view frustum's projection matrix.
	m_viewFrustum.SetProjectionMatrix( projection );

	// Create the list of render caches.
	m_renderCaches = new LinkedList< RenderCache >;

	// Search the mesh for unique materials.
	for( unsigned long m = 0; m < m_mesh->GetStaticMesh()->NumMaterials; m++ )
	{
		// Flag to determine if the material has been found already.
		bool found = false;

		// Ensure the new material is valid.
		if( m_mesh->GetStaticMesh()->materials[m] == NULL )
			continue;

		// Iterate through the already existing render caches.
		m_renderCaches->Iterate( true );
		while( m_renderCaches->Iterate() )
		{
			// Check if the new material already exists.
			if( m_renderCaches->GetCurrent()->GetMaterial() == m_mesh->GetStaticMesh()->materials[m] )
			{
				found = true;
				break;
			}
		}

		// Add the material if it wasn't found and not set to ignore faces.
		if( found == false && m_mesh->GetStaticMesh()->materials[m]->GetIgnoreFace() == false )
			m_renderCaches->Add( new RenderCache( g_engine->GetDevice(), m_mesh->GetStaticMesh()->materials[m] ) );
	}

	// Get a pointer to the mesh's frame list.
	LinkedList< Frame > *frames = m_mesh->GetFrameList();

	// Iterate through the frame list.
	frames->Iterate( true );
	while( frames->Iterate() != NULL )
	{
		// Check if this frame contains an occluder.
		if( strncmp( "oc_", frames->GetCurrent()->Name, 3 ) == 0 )
		{
			// If so, load the occluder, and continue to the next frame.
			m_occludingObjects->Add( new SceneOccluder( frames->GetCurrent()->GetTranslation(), ( (MeshContainer*)frames->GetCurrent()->pMeshContainer )->originalMesh, &frames->GetCurrent()->finalTransformationMatrix ) );
			continue;
		}

		// Check if this frame is a spawn point.
		if( strncmp( "sp_", frames->GetCurrent()->Name, 3 ) == 0 )
		{
			// Get the actual name of the spawner object at this spawn point.
			char *firstDash = strpbrk( frames->GetCurrent()->Name, "_" );
			firstDash++;
			char *lastDash = strrchr( firstDash, '_' );
			unsigned long length = lastDash - firstDash;
			char *name = new char[length + 5];
			ZeroMemory( name, sizeof( char ) * ( length + 5 ) );
			strncpy( name, firstDash, length );
			strcat( name, ".txt" );

			// Check if it is a player spawn point.
			if( _stricmp( name, "player.txt" ) == 0 )
			{
				// Get the name of the player spawn point's radius frame.
				char *radiusName = new char[strlen( firstDash ) + 8];
				ZeroMemory( radiusName, sizeof( char ) * ( strlen( firstDash ) + 8 ) );
				strncpy( radiusName, firstDash, strlen( firstDash ) );
				strcat( radiusName, "_radius" );

				// Find the player spawn point's radius frame.
				Frame *radiusFrame = frames->GetFirst();
				while( radiusFrame != NULL )
				{
					if( _stricmp( radiusFrame->Name, radiusName ) == 0 )
						break;

					radiusFrame = frames->GetNext( radiusFrame );
				}

				// Destroy the string buffer for the radius frame's name.
				SAFE_DELETE_ARRAY( radiusName );

				// Find the distance between the two points (the radius).
				float radius = 0.0f;
				if( radiusFrame != NULL )
					radius = D3DXVec3Length( &( radiusFrame->GetTranslation() - frames->GetCurrent()->GetTranslation() ) );

				// Create the player spawn point.
				SceneObject *point = new SceneObject( NULL, NULL );
				point->SetTranslation( frames->GetCurrent()->GetTranslation() );
				point->SetBoundingSphere( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), radius );
				point->SetVisible( false );
				point->SetGhost( true );
				point->Update( 0.0f );
				m_dynamicObjects->Add( m_playerSpawnPoints->Add( point ) );
			}
			else
			{
				// Create the application specific spawner object.
				SpawnerObject *spawner = new SpawnerObject( name, m_spawnerPath );
				spawner->SetTranslation( frames->GetCurrent()->GetTranslation() );
				spawner->Update( 0.0f );
				m_dynamicObjects->Add( m_objectSpawners->Add( spawner ) );
			}

			// Destroy the string buffer used to create the spawner's name.
			SAFE_DELETE_ARRAY( name );
		}
	}

	// A list of valid faces (those with a valid material).
	bool *validFaces = new bool[m_mesh->GetStaticMesh()->originalMesh->GetNumFaces()];
	ZeroMemory( validFaces, sizeof( bool ) * m_mesh->GetStaticMesh()->originalMesh->GetNumFaces() );

	// These are used for locking the vertex, index, and attribute buffers of
	// the meshes. They act as pointers into the data returned by the locks.
	Vertex *vertices = NULL;
	unsigned short *indices = NULL;
	unsigned long *attributes = NULL;

	// Lock the mesh's vertex, index, and attribute buffers.
	m_mesh->GetStaticMesh()->originalMesh->LockVertexBuffer( D3DLOCK_READONLY, (void**)&vertices );
	m_mesh->GetStaticMesh()->originalMesh->LockIndexBuffer( D3DLOCK_READONLY, (void**)&indices );
	m_mesh->GetStaticMesh()->originalMesh->LockAttributeBuffer( D3DLOCK_READONLY, &attributes );

	// Count the faces in the scene that have a valid material.
	for( unsigned long f = 0; f < m_mesh->GetStaticMesh()->originalMesh->GetNumFaces(); f++ )
	{
		m_renderCaches->Iterate( true );
		while( m_renderCaches->Iterate() )
		{
			if( m_renderCaches->GetCurrent()->GetMaterial() == m_mesh->GetStaticMesh()->materials[attributes[f]] )
			{
				m_totalFaces++;
				validFaces[f] = true;
				break;
			}
		}
	}

	// Create the array of faces.
	m_faces = new SceneFace[m_totalFaces];

	// Set the number of vertices.
	m_totalVertices = m_totalFaces * 3;

	// Create the vertex buffer that will hold all the vertices in the scene.
	g_engine->GetDevice()->CreateVertexBuffer( m_totalVertices * VERTEX_FVF_SIZE, D3DUSAGE_WRITEONLY, VERTEX_FVF, D3DPOOL_MANAGED, &m_sceneVertexBuffer, NULL );

	// Used for temporary storage of the final vertices that make the scene.
	Vertex *tempVertices = new Vertex[m_totalVertices];

	// Lock the scene's vertex buffer
	m_sceneVertexBuffer->Lock( 0, 0, (void**)&m_vertices, 0 );

	// Go through all of the valid faces in the mesh and store their details.
	for(int f = 0; f < (int)m_totalFaces; f++ )
	{
		// Ensure this face is valid.
		if( validFaces[f] == false )
		{
			// Advance the indices pointer to skip this face.
			indices += 3;
			continue;
		}

		// Store the index pointer for each vertex in the face.
		m_faces[f].vertex0 = *indices++;
		m_faces[f].vertex1 = *indices++;
		m_faces[f].vertex2 = *indices++;

		// Find the render cache this face belongs to.
		m_renderCaches->Iterate( true );
		while( m_renderCaches->Iterate() )
		{
			if( m_renderCaches->GetCurrent()->GetMaterial() == m_mesh->GetStaticMesh()->materials[attributes[f]] )
			{
				m_faces[f].renderCache = m_renderCaches->GetCurrent();
				m_renderCaches->GetCurrent()->AddFace();
				break;
			}
		}

		// Take of temporary copy of these vertices.
		tempVertices[m_faces[f].vertex0] = vertices[m_faces[f].vertex0];
		tempVertices[m_faces[f].vertex1] = vertices[m_faces[f].vertex1];
		tempVertices[m_faces[f].vertex2] = vertices[m_faces[f].vertex2];
	}

	// Copy the final vertices (that make up the scene) into the vertex buffer.
	memcpy( m_vertices, tempVertices, m_totalVertices * VERTEX_FVF_SIZE );

	// Unlock the scene's vertex buffer.
	m_sceneVertexBuffer->Unlock();

	// Destroy the temporary vertices array.
	SAFE_DELETE_ARRAY( tempVertices );

	// Unlock the mesh's vertex, index, and attribute buffers.
	m_mesh->GetStaticMesh()->originalMesh->UnlockAttributeBuffer();
	m_mesh->GetStaticMesh()->originalMesh->UnlockIndexBuffer();
	m_mesh->GetStaticMesh()->originalMesh->UnlockVertexBuffer();

	// Destroy the array of valid faces.
	SAFE_DELETE_ARRAY( validFaces );

	// Create the first scene leaf (the largest leaf that encloses the scene).
	m_firstLeaf = new SceneLeaf();

	// Recursively build the scene, starting with the first leaf.
	RecursiveSceneBuild( m_firstLeaf, m_mesh->GetBoundingSphere()->centre, m_mesh->GetBoundingBox()->halfSize );

	// Allow the render caches to prepare themselves.
	m_renderCaches->Iterate( true );
	while( m_renderCaches->Iterate() )
		m_renderCaches->GetCurrent()->Prepare( m_totalVertices );

	// Indicate that the scene is now loaded.
	m_loaded = true;
}

//-----------------------------------------------------------------------------
// Destroys the currently loaded scene.
//-----------------------------------------------------------------------------
void SceneManager::DestroyScene()
{
	// Destroy the array of polygons.
	SAFE_DELETE_ARRAY( m_faces );
	m_totalFaces = 0;

	// Destroy the list of render caches.
	SAFE_DELETE( m_renderCaches );

	// Release the scene's vertex buffer.
	SAFE_RELEASE( m_sceneVertexBuffer );
	m_vertices = NULL;
	m_totalVertices = 0;

	// Destroy the scene leaf hierarchy.
	SAFE_DELETE( m_firstLeaf );

	// Destroy the object spawner list.
	if( m_objectSpawners != NULL )
		m_objectSpawners->ClearPointers();
	SAFE_DELETE( m_objectSpawners );

	// Destroy the list of player spawn points.
	if( m_playerSpawnPoints != NULL )
		m_playerSpawnPoints->ClearPointers();
	SAFE_DELETE( m_playerSpawnPoints );

	// Destroy the lists of occluding objects.
	if( m_visibleOccluders != NULL )
		m_visibleOccluders->ClearPointers();
	SAFE_DELETE( m_visibleOccluders );
	SAFE_DELETE( m_occludingObjects );

	// Empty the list of dynamic objects.
	m_dynamicObjects->Empty();

	// Destroy the scene's mesh.
	g_engine->GetMeshManager()->Remove( &m_mesh );

	// Destroy the scene name.
	SAFE_DELETE_ARRAY( m_name );

	// Indicate that the scene is no longer loaded.
	m_loaded = false;
}

//-----------------------------------------------------------------------------
// Returns true if the scene is loaded and ready for use.
//-----------------------------------------------------------------------------
bool SceneManager::IsLoaded()
{
	return m_loaded;
}

//-----------------------------------------------------------------------------
// Updates the scene and all the objects in it.
//-----------------------------------------------------------------------------
void SceneManager::Update( float elapsed, D3DXMATRIX *view )
{
	// Ensure a scene is loaded.
	if( m_firstLeaf == NULL )
		return;

	// Increment the frame stamp. Indicating the start of a new frame.
	m_frameStamp++;

	// Update the view frustum.
	m_viewFrustum.Update( view );

	// Go through all the dynamic object's and update them.
	m_dynamicObjects->Iterate( true );
	while( m_dynamicObjects->Iterate() )
	{
		// Ignore the object if it is not enabled.
		if( m_dynamicObjects->GetCurrent()->GetEnabled() == false )
			continue;

		// If this object is a ghost, then it cannot collided with anything.
		// However, it still needs to be updated. Since objects receive their
		// movement through the collision system, ghost objects will have to be
		// allowed to update their movement manually.
		if( m_dynamicObjects->GetCurrent()->GetGhost() == true )
		{
			m_dynamicObjects->GetCurrent()->Update( elapsed );
			continue;
		}

		// Objects without an ellipsoid radius cannot collide with anything.
		if( m_dynamicObjects->GetCurrent()->GetEllipsoidRadius().x + m_dynamicObjects->GetCurrent()->GetEllipsoidRadius().y, m_dynamicObjects->GetCurrent()->GetEllipsoidRadius().z <= 0.0f )
		{
			m_dynamicObjects->GetCurrent()->Update( elapsed );
			continue;
		}

		// Build the collision data for this object.
		static CollisionData collisionData;
		collisionData.scale = m_scale;
		collisionData.elapsed = elapsed;
		collisionData.frameStamp = m_frameStamp;
		collisionData.object = m_dynamicObjects->GetCurrent();
		collisionData.gravity = m_gravity * elapsed;

		// Perform collision detection for this object.
		PerformCollisionDetection( &collisionData, (Vertex*)m_vertices, m_faces, m_totalFaces, m_dynamicObjects );

		// Allow the object to update itself.
		m_dynamicObjects->GetCurrent()->Update( elapsed, false );
	}
}

//-----------------------------------------------------------------------------
// Renders the scene and all the objects in it.
//-----------------------------------------------------------------------------
void SceneManager::Render( float elapsed, D3DXVECTOR3 viewer )
{
	// Ensure a scene is loaded.
	if( m_firstLeaf == NULL )
		return;

	// Clear the list of visible occluders.
	m_visibleOccluders->ClearPointers();

	// Begin the process of determining the visible leaves in the scene. The
	// first step involves checking the scene leaves against the view frustum.
	RecursiveSceneFrustumCheck( m_firstLeaf, viewer );

	// A list of potentially visible leaves and occluders has been determined
	// after check against the view frustum. The next step is to go through and
	// further refine the list of visible occluders through occlusion culling.
	m_visibleOccluders->Iterate( true );
	while( m_visibleOccluders->Iterate() )
	{
		// If the occluder's visible stamp does not not equal the current frame
		// stamp then the occluder has been hidden somehow, so ignore it.
		if( m_visibleOccluders->GetCurrent()->visibleStamp != m_frameStamp )
			continue;

		// Build the occluder's occlusion volume.
		BuildOcclusionVolume( m_visibleOccluders->GetCurrent(), viewer );

		// Iterate through the rest of the visible occluder's.
		SceneOccluder *occludee = m_visibleOccluders->GetNext( m_visibleOccluders->GetCurrent() );
		while( occludee != NULL )
		{
			// If the occludee's bounding sphere is overlapping the occluder's
			// volume and the occludee's bounding box is completely enclosed by
			// the occluder's volume, then the occludee is hidden.
			if( IsSphereOverlappingVolume( m_visibleOccluders->GetCurrent()->planes, occludee->translation, occludee->GetBoundingSphere()->radius ) == true )
				if( IsBoxEnclosedByVolume( m_visibleOccluders->GetCurrent()->planes, occludee->GetBoundingBox()->min, occludee->GetBoundingBox()->max ) == true )
					occludee->visibleStamp--;

			occludee = m_visibleOccluders->GetNext( occludee );
		}
	}

	// Tell all the render caches to prepare for rendering.
	m_renderCaches->Iterate( true );
	while( m_renderCaches->Iterate() )
		m_renderCaches->GetCurrent()->Begin();

	// Finally, check the scene's leaves against the visible occluders.
	RecursiveSceneOcclusionCheck( m_firstLeaf );

	// Set an identity world transformation matrix to render around the origin.
	D3DXMATRIX world;
	D3DXMatrixIdentity( &world );
	g_engine->GetDevice()->SetTransform( D3DTS_WORLD, &world );

	// Set the scene vertex buffer as the current device data stream.
	g_engine->GetDevice()->SetStreamSource( 0, m_sceneVertexBuffer, 0, VERTEX_FVF_SIZE );
	g_engine->GetDevice()->SetFVF( VERTEX_FVF );

	// Tell all the render caches to end rendering. This will cause them to
	// send their faces to be render to the video card.
	m_renderCaches->Iterate( true );
	while( m_renderCaches->Iterate() )
		m_renderCaches->GetCurrent()->End();

	// Iterate through the list of dynamic objects.
	m_dynamicObjects->Iterate( true );
	while( m_dynamicObjects->Iterate() )
	{
		// Check if the object is visible.
		if( m_dynamicObjects->GetCurrent()->GetVisible() == false )
			continue;

		// Check if the object's bounding sphere is inside the view frustum.
		if( m_viewFrustum.IsSphereInside( m_dynamicObjects->GetCurrent()->GetBoundingSphere()->centre, m_dynamicObjects->GetCurrent()->GetBoundingSphere()->radius ) == false )
			continue;

		// Iterate through the list of visible occluders.
		bool occluded = false;
		m_visibleOccluders->Iterate( true );
		while( m_visibleOccluders->Iterate() )
		{
			// Ignore hidden occluders.
			if( m_visibleOccluders->GetCurrent()->visibleStamp != m_frameStamp )
				continue;

			occluded = true;

			// Check the object's bounding sphere against the occlusion volume.
			m_visibleOccluders->GetCurrent()->planes->Iterate( true );
			while( m_visibleOccluders->GetCurrent()->planes->Iterate() )
			{
				if( D3DXPlaneDotCoord( m_visibleOccluders->GetCurrent()->planes->GetCurrent(), &m_dynamicObjects->GetCurrent()->GetBoundingSphere()->centre ) < m_dynamicObjects->GetCurrent()->GetBoundingSphere()->radius )
				{
					occluded = false;
					break;
				}
			}

			// Break if the object is completely hidden by this occluder.
			if( occluded == true )
				break;
		}

		// Ignore this object if it is occluded.
		if( occluded == true )
			continue;

		// Render the object.
		m_dynamicObjects->GetCurrent()->Render();
	}
}

//-----------------------------------------------------------------------------
// Adds the given object to the scene.
//-----------------------------------------------------------------------------
SceneObject *SceneManager::AddObject( SceneObject *object )
{
	return m_dynamicObjects->Add( object );
}

//-----------------------------------------------------------------------------
// Removes the given object from the scene.
//-----------------------------------------------------------------------------
void SceneManager::RemoveObject( SceneObject **object )
{
	m_dynamicObjects->ClearPointer( object );
}

//-----------------------------------------------------------------------------
// Returns a random player spawnpoint.
//-----------------------------------------------------------------------------
SceneObject *SceneManager::GetRandomPlayerSpawnPoint()
{
	// Get a random spawn point.
	SceneObject *point = m_playerSpawnPoints->GetRandom();

	// If the spawner's collision stamp equals the current frame stamp, then
	// something has collided with the spawner. Alternatively this spawner may
	// not be enabled. In either case, return NULL to indicate that a vacent
	// spawn point was not found. The caller will have to try again later.
	if( point->GetCollisionStamp() != m_frameStamp && point->GetEnabled() == true )
		return point;
	else
		return NULL;
}

//-----------------------------------------------------------------------------
// Returns the spawn point with the given ID (position in the linked list).
//-----------------------------------------------------------------------------
SceneObject *SceneManager::GetSpawnPointByID( long id )
{
	SceneObject *point = NULL;

	// Ensure the ID is in range.
	if( id < (long)m_playerSpawnPoints->GetTotalElements() )
	{
		// Loop through the player spawn point list until the id is reached.
		point = m_playerSpawnPoints->GetFirst();
		for( long i = 0; i < id; i++ )
			point = m_playerSpawnPoints->GetNext( point );
	}

	return point;
}

//-----------------------------------------------------------------------------
// Returns the ID (position in the linked list) of the given spawn point.
//-----------------------------------------------------------------------------
long SceneManager::GetSpawnPointID( SceneObject *point )
{
	// Ensure the given spawn point is valid.
	if( point == NULL )
		return -1;

	long id = 0;

	// Iterate the player spawn point list looking for the given spawn point.
	m_playerSpawnPoints->Iterate( true );
	while( m_playerSpawnPoints->Iterate() != NULL )
	{
		if( m_playerSpawnPoints->GetCurrent() == point )
			break;

		id++;
	}

	// If the ID equals the total elements, then the spawn point was not found.
	long TotalElements = (long)m_playerSpawnPoints->GetTotalElements();
	if( id == TotalElements )
		id = -1;

	return id;
}

//-----------------------------------------------------------------------------
// Returns the list of object spawners in the scene.
//-----------------------------------------------------------------------------
LinkedList< SpawnerObject > *SceneManager::GetSpawnerObjectList()
{
	return m_objectSpawners;
}

//-----------------------------------------------------------------------------
// Returns the result of a ray intersection with the scene and all its objects.
//-----------------------------------------------------------------------------
bool SceneManager::RayIntersectScene( RayIntersectionResult *result, D3DXVECTOR3 rayPosition, D3DXVECTOR3 rayDirection, bool checkScene, SceneObject *thisObject, bool checkObjects )
{
	float hitDistance = 0.0f;

	// Check if the ray needs to check for intersection with the scene.
	if( checkScene == true )
	{
		// Go through all the faces in the scene, check for intersection.
		for( unsigned long f = 0; f < m_totalFaces; f++ )
		{
			// Skip this face if its material is set to ignore rays.
			if( m_faces[f].renderCache->GetMaterial()->GetIgnoreRay() == true )
				continue;

			// Check the ray against this face.
			if( D3DXIntersectTri( (D3DXVECTOR3*)&m_vertices[m_faces[f].vertex0], (D3DXVECTOR3*)&m_vertices[m_faces[f].vertex1], (D3DXVECTOR3*)&m_vertices[m_faces[f].vertex2], &rayPosition, &rayDirection, NULL, NULL, &hitDistance ) == TRUE )
			{
				if( hitDistance < result->distance || result->material == NULL )
				{
					( *result ).distance = hitDistance;
					( *result ).material = m_faces[f].renderCache->GetMaterial();
				}
			}
		}
	}

	// Check if the ray needs to check for intersection with the objects.
	if( checkObjects == true )
	{
		// Stores the ray in model space.
		D3DXVECTOR3 rp, rd;

		// Iterate all the objects in the scene, check for intersection.
		SceneObject *object = m_dynamicObjects->GetFirst();
		while( object != NULL )
		{
			// Only check this object if it is enabled, has a mesh and is not
			// the calling object.
			if( object->GetEnabled() == true && object->GetMesh() != NULL && object != thisObject )
			{
				// Transform the ray into model space.
				D3DXMATRIX inverse;
				D3DXMatrixInverse( &inverse, NULL, object->GetWorldMatrix() );
				D3DXVec3TransformCoord( &rp, &rayPosition, &inverse );
				D3DXVec3TransformNormal( &rd, &rayDirection, &inverse );

				// Go through the list of frames in the object's mesh.
				LinkedList< Frame > *frames = object->GetMesh()->GetFrameList();
				frames->Iterate( true );
				while( frames->Iterate() != NULL )
				{
					// Ignore this frame if it has no mesh.
					if( frames->GetCurrent()->pMeshContainer == NULL )
						continue;

					// Check the ray against this frame's mesh.
					BOOL hit;
					D3DXIntersect( frames->GetCurrent()->pMeshContainer->MeshData.pMesh, &rp, &rd, &hit, NULL, NULL, NULL, &hitDistance, NULL, NULL );
					if( hit == TRUE && ( hitDistance < result->distance || result->material == NULL ) )
					{
						( *result ).distance = hitDistance;
						( *result ).material = object->GetMesh()->GetStaticMesh()->materials[0];
						( *result ).hitObject = object;
					}
				}
			}

			// Go to the next object.
			object = m_dynamicObjects->GetNext( object );
		}
	}

	// Return false if no intersection occured.
	if( result->material == NULL )
		return false;

	// Calculate the point of intersection.
	( *result ).point = rayPosition + rayDirection * result->distance;

	return true;
}

//-----------------------------------------------------------------------------
// Builds an occlusion volume for the given occluder.
//-----------------------------------------------------------------------------
void SceneManager::BuildOcclusionVolume( SceneOccluder *occluder, D3DXVECTOR3 viewer )
{
	// Create a list of edges for the occluder's silhouette.
	LinkedList< Edge > *edges = new LinkedList< Edge >;

	// Go through all the faces in the occluder's mesh.
	for( unsigned long f = 0; f < occluder->totalFaces; f++ )
	{
		// Get the indices of this face.
		unsigned short index0 = occluder->indices[3 * f + 0];
		unsigned short index1 = occluder->indices[3 * f + 1];
		unsigned short index2 = occluder->indices[3 * f + 2];

		// Find the angle between the face's normal and the vector point from
		// viewer's position to the face's position. If the angle is less than
		// 0, then the face is visible to the viewer.
		if( D3DXVec3Dot( &occluder->vertices[index0].normal, &( occluder->vertices[index0].translation - viewer ) ) < 0.0f )
		{
			// Check if the list of edges is empty.
			if( edges->GetTotalElements() == 0 )
			{
				// Add all the edges for this face.
				edges->Add( new Edge( &occluder->vertices[index0], &occluder->vertices[index1] ) );
				edges->Add( new Edge( &occluder->vertices[index1], &occluder->vertices[index2] ) );
				edges->Add( new Edge( &occluder->vertices[index2], &occluder->vertices[index0] ) );
			}
			else
			{
				Edge *found0 = NULL;
				Edge *found1 = NULL;
				Edge *found2 = NULL;

				// Iterate through the list of edges.
				edges->Iterate( true );
				while( edges->Iterate() != NULL )
				{
					// Check if the first edge of this face already exists.
					if( ( edges->GetCurrent()->vertex0->translation == occluder->vertices[index0].translation && edges->GetCurrent()->vertex1->translation == occluder->vertices[index1].translation ) ||
						( edges->GetCurrent()->vertex0->translation == occluder->vertices[index1].translation && edges->GetCurrent()->vertex1->translation == occluder->vertices[index0].translation ) )
						found0 = edges->GetCurrent();

					// Check if the second edge of this face already exists.
					if( ( edges->GetCurrent()->vertex0->translation == occluder->vertices[index1].translation && edges->GetCurrent()->vertex1->translation == occluder->vertices[index2].translation ) ||
						( edges->GetCurrent()->vertex0->translation == occluder->vertices[index2].translation && edges->GetCurrent()->vertex1->translation == occluder->vertices[index1].translation ) )
						found1 = edges->GetCurrent();

					// Check if the third edge of this face already exists.
					if( ( edges->GetCurrent()->vertex0->translation == occluder->vertices[index2].translation && edges->GetCurrent()->vertex1->translation == occluder->vertices[index0].translation ) ||
						( edges->GetCurrent()->vertex0->translation == occluder->vertices[index0].translation && edges->GetCurrent()->vertex1->translation == occluder->vertices[index2].translation ) )
						found2 = edges->GetCurrent();
				}

				// If the first edge was found, remove it. Otherwise add it.
				if( found0 != NULL )
					edges->Remove( &found0 );
				else
					edges->Add( new Edge( &occluder->vertices[index0], &occluder->vertices[index1] ) );

				// If the second edge was found, remove it. Otherwise add it.
				if( found1 != NULL )
					edges->Remove( &found1 );
				else
					edges->Add( new Edge( &occluder->vertices[index1], &occluder->vertices[index2] ) );

				// If the thrid edge was found, remove it. Otherwise add it.
				if( found2 != NULL )
					edges->Remove( &found2 );
				else
					edges->Add( new Edge( &occluder->vertices[index2], &occluder->vertices[index0] ) );
			}
		}
	}

	// Empty the occluder's list of planes.
	occluder->planes->Empty();

	// Create the front cap plane.
	D3DXPLANE *plane = new D3DXPLANE;
	D3DXPlaneFromPointNormal( plane, &occluder->translation, &( occluder->translation - viewer ) );
	occluder->planes->Add( plane );

	// Iterate through the list of edges.
	edges->Iterate( true );
	while( edges->Iterate() != NULL )
	{
		// Get the position of the vertices in the edge.
		D3DXVECTOR3 vertex1 = edges->GetCurrent()->vertex0->translation;
		D3DXVECTOR3 vertex2 = edges->GetCurrent()->vertex1->translation;

		// Calculate the position of the thrid vertex for creating the plane.
		D3DXVECTOR3 dir = vertex1 - viewer;
		D3DXVec3Normalize( &dir, &dir );
		D3DXVECTOR3 vertex3 = vertex1 + dir;

		// Create a plane from this edge.
		plane = new D3DXPLANE;
		D3DXPlaneFromPoints( plane, &vertex1, &vertex2, &vertex3 );
		occluder->planes->Add( plane );
	}

	// Destroy the list of edges.
	SAFE_DELETE( edges );
}

//-----------------------------------------------------------------------------
// Recursively builds the scene.
//-----------------------------------------------------------------------------
void SceneManager::RecursiveSceneBuild( SceneLeaf *leaf, D3DXVECTOR3 translation, float halfSize )
{
	// Build a bounding volume around this leaf.
	leaf->SetBoundingBox( D3DXVECTOR3( translation.x - halfSize, translation.y - halfSize, translation.z - halfSize ), D3DXVECTOR3( translation.x + halfSize, translation.y + halfSize, translation.z + halfSize ) );
	leaf->SetBoundingSphere( translation, (float)sqrt( halfSize * halfSize + halfSize * halfSize + halfSize * halfSize ) );

	// Count the number of face in this leaf.
	unsigned long totalFaces = 0;
	for( unsigned long f = 0; f < m_totalFaces; f++ )
		if( IsFaceInBox( &m_vertices[m_faces[f].vertex0], &m_vertices[m_faces[f].vertex1], &m_vertices[m_faces[f].vertex2], leaf->GetBoundingBox()->min, leaf->GetBoundingBox()->max ) == true )
			totalFaces++;

	// Only divide the leaf up if it is too big and contains too many faces.
	if( halfSize > m_maxHalfSize && totalFaces > m_maxFaces )
	{
		// Go through all the child leaves.
		for( char c = 0; c < 8; c++ )
		{
			D3DXVECTOR3 newTranslation, newMin, newMax;
			float newHalfSize = halfSize / 2.0f;
			float mod;

			// Calculate the translation of the new leaf on the x axis.
			mod = 1.0f;
			if( c % 2 < 1 )
				mod = -1.0f;
			newTranslation.x = translation.x + newHalfSize * mod;

			// Calculate the translation of the new leaf on the y axis.
			mod = 1.0f;
			if( c % 4 < 2 )
				mod = -1.0f;
			newTranslation.y = translation.y + newHalfSize * mod;

			// Calculate the translation of the new leaf on the z axis.
			mod = 1.0f;
			if( c % 8 < 4 )
				mod = -1.0f;
			newTranslation.z = translation.z + newHalfSize * mod;

			// Calculate the bounding box around the new leaf.
			newMin = D3DXVECTOR3( newTranslation.x - newHalfSize, newTranslation.y - newHalfSize, newTranslation.z - newHalfSize );
			newMax = D3DXVECTOR3( newTranslation.x + newHalfSize, newTranslation.y + newHalfSize, newTranslation.z + newHalfSize );

			// Check if the new scene leaf will have at least one face in it.
			for(int f = 0; f < (int)m_totalFaces; f++ )
			{
				if( IsFaceInBox( &m_vertices[m_faces[f].vertex0], &m_vertices[m_faces[f].vertex1], &m_vertices[m_faces[f].vertex2], newMin, newMax ) == true )
				{
					// A face has been found that is inside the new child scene
					// leaf, so create the scene leaf. Then recurse through the
					// scene leaf's branch of the scene hierarchy.
					leaf->children[c] = new SceneLeaf;
					RecursiveSceneBuild( leaf->children[c], newTranslation, halfSize / 2.0f );

					break;
				}
			}
		}

		return;
	}

	// Create the leaf's array of face indices.
	leaf->totalFaces = totalFaces;
	leaf->faces = new unsigned long[totalFaces];

	// If any face is contained in the leaf's bounding box, then store
	// the index of the face in the leaf's face index array.
	totalFaces = 0;
	for(int f = 0; f < (int)m_totalFaces; f++ )
		if( IsFaceInBox( &m_vertices[m_faces[f].vertex0], &m_vertices[m_faces[f].vertex1], &m_vertices[m_faces[f].vertex2], leaf->GetBoundingBox()->min, leaf->GetBoundingBox()->max ) == true )
			leaf->faces[totalFaces++] = f;

	// Store pointers to any occluding objects in the leaf.
	m_occludingObjects->Iterate( true );
	while( m_occludingObjects->Iterate() )
		if( IsBoxInBox( m_occludingObjects->GetCurrent()->GetBoundingBox()->min, m_occludingObjects->GetCurrent()->GetBoundingBox()->max, leaf->GetBoundingBox()->min, leaf->GetBoundingBox()->max ) == true )
			leaf->occluders->Add( m_occludingObjects->GetCurrent() );
}

//-----------------------------------------------------------------------------
// Recursively checks the scene's leaves against the view frustum.
//-----------------------------------------------------------------------------
bool SceneManager::RecursiveSceneFrustumCheck( SceneLeaf *leaf, D3DXVECTOR3 viewer )
{
	// Check if the leaf's bounding sphere is inside the view frustum.
	if( m_viewFrustum.IsSphereInside( leaf->GetBoundingSphere()->centre, leaf->GetBoundingSphere()->radius ) == false )
		return false;

	// Check if the leaf's bounding box is inside the view frustum.
	if( m_viewFrustum.IsBoxInside( leaf->GetBoundingBox()->min, leaf->GetBoundingBox()->max ) == false )
		return false;

	// Set the visible stamp on this leaf to the current frame stamp. This will
	// indicate that the leaf may be visible this frame and may need rendering.
	leaf->visibleStamp = m_frameStamp;

	// Check if any of this leaf's children are visible.
	char visibleChildren = 0;
	for( char c = 0; c < 8; c++ )
		if( leaf->children[c] != NULL )
			if( RecursiveSceneFrustumCheck( leaf->children[c], viewer ) )
				visibleChildren++;

	// If this leaf has visible children then this branch of the scene can go
	// deeper. So ignore this leaf.
	if( visibleChildren > 0 )
		return false;

	// Iterate through all the occluders in this leaf.
	leaf->occluders->Iterate( true );
	while( leaf->occluders->Iterate() )
	{
		// Check if the occluder's bounding sphere is inside the view frustum.
		if( m_viewFrustum.IsSphereInside( leaf->occluders->GetCurrent()->translation, leaf->occluders->GetCurrent()->GetBoundingSphere()->radius ) == false )
			continue;

		// Check if the occluder's bounding box is inside the view frustum.
		if( m_viewFrustum.IsBoxInside( leaf->occluders->GetCurrent()->GetBoundingBox()->min, leaf->occluders->GetCurrent()->GetBoundingBox()->max ) == false )
			continue;

		// Calculate the distance between the occluder and the viewer.
		leaf->occluders->GetCurrent()->distance = D3DXVec3Length( &( leaf->occluders->GetCurrent()->translation - viewer ) );

		// Iterate through the list of visible occluders.
		m_visibleOccluders->Iterate( true );
		while( m_visibleOccluders->Iterate() )
		{
			// If the new occluder is already in the list, don't add it agian.
			if( leaf->occluders->GetCurrent() == m_visibleOccluders->GetCurrent() )
				break;

			// If the new occluder is closer to the viewer than this occluder,
			// then add it to the list before this occluder.
			if( leaf->occluders->GetCurrent()->distance < m_visibleOccluders->GetCurrent()->distance )
			{
				m_visibleOccluders->InsertBefore( leaf->occluders->GetCurrent(), m_visibleOccluders->GetCompleteElement( m_visibleOccluders->GetCurrent() ) );
				leaf->occluders->GetCurrent()->visibleStamp = m_frameStamp;
				break;
			}
		}

		// If the occluder wasn't in the list or not added then add it now.
		if( leaf->occluders->GetCurrent()->visibleStamp != m_frameStamp )
		{
			m_visibleOccluders->Add( leaf->occluders->GetCurrent() );
			leaf->occluders->GetCurrent()->visibleStamp = m_frameStamp;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Recursively checks the scene's leaves against the occlusion volumes.
//-----------------------------------------------------------------------------
void SceneManager::RecursiveSceneOcclusionCheck( SceneLeaf *leaf )
{
	// Ignore the leaf if it is not visible this frame.
	if( leaf->visibleStamp != m_frameStamp )
		return;

	// Iterate through the list of visible occluders.
	m_visibleOccluders->Iterate( true );
	while( m_visibleOccluders->Iterate() )
	{
		// Ignore hidden occluders.
		if( m_visibleOccluders->GetCurrent()->visibleStamp != m_frameStamp )
			continue;

		// If the leaf's bounding sphere is overlapping the occluder's volume
		// and the leaf's bounding box is completely enclosed by the occluder's
		// volume, then the leaf is hidden, so ignore it.
		if( IsSphereOverlappingVolume( m_visibleOccluders->GetCurrent()->planes, leaf->GetBoundingSphere()->centre, leaf->GetBoundingSphere()->radius ) == true )
			if( IsBoxEnclosedByVolume( m_visibleOccluders->GetCurrent()->planes, leaf->GetBoundingBox()->min, leaf->GetBoundingBox()->max ) == true )
				return;
	}

	// Check if any of this leaf's children are visible.
	for( char c = 0; c < 8; c++ )
		if( leaf->children[c] != NULL )
			RecursiveSceneOcclusionCheck( leaf->children[c] );

	// Go through all the faces in the leaf.
	for( unsigned long f = 0; f < leaf->totalFaces; f++ )
	{
		// Check this face's render stamp. If it is equal to the current frame
		// stamp, then the face has already been rendered this frame.
		if( m_faces[leaf->faces[f]].renderStamp == m_frameStamp )
			continue;

		// Set the face's render stamp to indicate that it has been rendered.
		m_faces[leaf->faces[f]].renderStamp = m_frameStamp;

		// Tell the face's render cache to render this face.
		m_faces[leaf->faces[f]].renderCache->RenderFace( m_faces[leaf->faces[f]].vertex0, m_faces[leaf->faces[f]].vertex1, m_faces[leaf->faces[f]].vertex2 );
	}
}